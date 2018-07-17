// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "clientversion.h"
#include "init.h"
#include "validation.h"
#include "net.h"
#include "netbase.h"
#include "rpc/server.h"
#include "timedata.h"
#include "util.h"
#include "utilstrencodings.h"
#include "validation.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#endif

#include "primitives/transaction.h"
#include <stdint.h>

#include <boost/assign/list_of.hpp>

#include <univalue.h>

using namespace std;

/**
 * @note Do not add or change anything in the information returned by this
 * method. `getinfo` exists for backwards-compatibility only. It combines
 * information from wildly different sources in the program, which is a mess,
 * and is thus planned to be deprecated eventually.
 *
 * Based on the source of the information, new information should be added to:
 * - `getblockchaininfo`,
 * - `getnetworkinfo` or
 * - `getwalletinfo`
 *
 * Or alternatively, create a specific query method for the information.
 **/
#define  NUM_OFDIS 50

UniValue getinfo(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 0)
        throw runtime_error(
            "getinfo\n"
            "\nDEPRECATED. Returns an object containing various state info.\n"
            "\nResult:\n"
            "{\n"
            "  \"version\": xxxxx,           (numeric) the server version\n"
            "  \"protocolversion\": xxxxx,   (numeric) the protocol version\n"
            "  \"walletversion\": xxxxx,     (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,         (numeric) the total ipchain balance of the wallet\n"
            "  \"blocks\": xxxxxx,           (numeric) the current number of blocks processed in the server\n"
            "  \"timeoffset\": xxxxx,        (numeric) the time offset\n"
            "  \"connections\": xxxxx,       (numeric) the number of connections\n"
            "  \"proxy\": \"host:port\",     (string, optional) the proxy used by the server\n"
            "  \"difficulty\": xxxxxx,       (numeric) the current difficulty\n"
            "  \"testnet\": true|false,      (boolean) if the server is using testnet or not\n"
			"  \"isbrowserable\": true|false,(boolean) Whether the browser query interface service is supported\n"
            "  \"keypoololdest\": xxxxxx,    (numeric) the timestamp (seconds since Unix epoch) of the oldest pre-generated key in the key pool\n"
            "  \"keypoolsize\": xxxx,        (numeric) how many new keys are pre-generated\n"
            "  \"unlocked_until\": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "  \"paytxfee\": x.xxxx,         (numeric) the transaction fee set in " + CURRENCY_UNIT + "/kB\n"
            "  \"relayfee\": x.xxxx,         (numeric) minimum relay fee for non-free transactions in " + CURRENCY_UNIT + "/kB\n"
            "  \"errors\": \"...\"           (string) any error messages\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getinfo", "")
            + HelpExampleRpc("getinfo", "")
        );

#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    proxyType proxy;
    GetProxy(NET_IPV4, proxy);

    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("version", CLIENT_VERSION));
    obj.push_back(Pair("protocolversion", PROTOCOL_VERSION));
#ifdef ENABLE_WALLET
    if (pwalletMain) {
        obj.push_back(Pair("walletversion", pwalletMain->GetVersion()));
        obj.push_back(Pair("balance",       ValueFromAmount(pwalletMain->GetBalance())));
    }
#endif
    obj.push_back(Pair("blocks",        (int)chainActive.Height()));
    obj.push_back(Pair("timeoffset",    GetTimeOffset()));
    if(g_connman)
        obj.push_back(Pair("connections",   (int)g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL)));
    obj.push_back(Pair("proxy",         (proxy.IsValid() ? proxy.proxy.ToStringIPPort() : string())));
    obj.push_back(Pair("difficulty",    (double)GetDifficulty()));
    obj.push_back(Pair("testnet",       Params().NetworkIDString() == CBaseChainParams::TESTNET));
	obj.push_back(Pair("isbrowserable", fAddressIndex ));
// 	if (b_TestTxLarge)
// 		obj.push_back(Pair("b_TestTxLarge",   b_TestTxLarge));
#ifdef ENABLE_WALLET
    if (pwalletMain) {
        obj.push_back(Pair("keypoololdest", pwalletMain->GetOldestKeyPoolTime()));
        obj.push_back(Pair("keypoolsize",   (int)pwalletMain->GetKeyPoolSize()));
    }
    if (pwalletMain && pwalletMain->IsCrypted())
        obj.push_back(Pair("unlocked_until", nWalletUnlockTime));
    obj.push_back(Pair("paytxfee",      ValueFromAmount(payTxFee.GetFeePerK())));
#endif
    obj.push_back(Pair("relayfee",      ValueFromAmount(::minRelayTxFee.GetFeePerK())));
    obj.push_back(Pair("errors",        GetWarnings("statusbar")));
    return obj;
}

UniValue getIPCversion(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() != 0)
		throw runtime_error(
		"getIPCversion\n"
		"\nDEPRECATED. Returns an object containing various state info.\n"
		"\nResult:\n"
		"{\n"
		"  \"version\": xxxxx,           (string) the IPChain version\n"
		"}\n"
		"\nExamples:\n"
		+ HelpExampleCli("getIPCversion", "")
		+ HelpExampleRpc("getIPCversion", "")
		);

#ifdef ENABLE_WALLET
	LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
	LOCK(cs_main);
#endif

	UniValue obj(UniValue::VOBJ);
	obj.push_back(Pair("version", IPCHAIN_VERSION));
	return obj;
}

#ifdef ENABLE_WALLET
class DescribeAddressVisitor : public boost::static_visitor<UniValue>
{
public:
    UniValue operator()(const CNoDestination &dest) const { return UniValue(UniValue::VOBJ); }

    UniValue operator()(const CKeyID &keyID) const {
        UniValue obj(UniValue::VOBJ);
        CPubKey vchPubKey;
        obj.push_back(Pair("isscript", false));
        if (pwalletMain && pwalletMain->GetPubKey(keyID, vchPubKey)) {
            obj.push_back(Pair("pubkey", HexStr(vchPubKey)));
            obj.push_back(Pair("iscompressed", vchPubKey.IsCompressed()));
        }
        return obj;
    }

    UniValue operator()(const CScriptID &scriptID) const {
        UniValue obj(UniValue::VOBJ);
        CScript subscript;
        obj.push_back(Pair("isscript", true));
        if (pwalletMain && pwalletMain->GetCScript(scriptID, subscript)) {
            std::vector<CTxDestination> addresses;
            txnouttype whichType;
            int nRequired;
            ExtractDestinations(subscript, whichType, addresses, nRequired);
            obj.push_back(Pair("script", GetTxnOutputType(whichType)));
            obj.push_back(Pair("hex", HexStr(subscript.begin(), subscript.end())));
            UniValue a(UniValue::VARR);
            BOOST_FOREACH(const CTxDestination& addr, addresses)
                a.push_back(CBitcoinAddress(addr).ToString());
            obj.push_back(Pair("addresses", a));
            if (whichType == TX_MULTISIG)
                obj.push_back(Pair("sigsrequired", nRequired));
        }
        return obj;
    }
};
#endif

UniValue validateaddress(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw runtime_error(
            "validateaddress \"address\"\n"
            "\nReturn information about the given ipchain address.\n"
            "\nArguments:\n"
            "1. \"address\"     (string, required) The ipchain address to validate\n"
            "\nResult:\n"
            "{\n"
            "  \"isvalid\" : true|false,       (boolean) If the address is valid or not. If not, this is the only property returned.\n"
            "  \"address\" : \"address\", (string) The ipchain address validated\n"
            "  \"scriptPubKey\" : \"hex\",       (string) The hex encoded scriptPubKey generated by the address\n"
            "  \"ismine\" : true|false,        (boolean) If the address is yours or not\n"
            "  \"iswatchonly\" : true|false,   (boolean) If the address is watchonly\n"
            "  \"isscript\" : true|false,      (boolean) If the key is a script\n"
            "  \"pubkey\" : \"publickeyhex\",    (string) The hex value of the raw public key\n"
            "  \"iscompressed\" : true|false,  (boolean) If the address is compressed\n"
            "  \"account\" : \"account\"         (string) DEPRECATED. The account associated with the address, \"\" is the default account\n"
            "  \"timestamp\" : timestamp,        (number, optional) The creation time of the key if available in seconds since epoch (Jan 1 1970 GMT)\n"
            "  \"hdkeypath\" : \"keypath\"       (string, optional) The HD keypath if the key is HD and available\n"
            "  \"hdmasterkeyid\" : \"<hash160>\" (string, optional) The Hash160 of the HD master pubkey\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("validateaddress", "\"1PSSGeFHDnKNxiEyFrD1wcEaHr9hrQDDWc\"")
            + HelpExampleRpc("validateaddress", "\"1PSSGeFHDnKNxiEyFrD1wcEaHr9hrQDDWc\"")
        );

#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    CBitcoinAddress address(request.params[0].get_str());
    bool isValid = address.IsValid();

    UniValue ret(UniValue::VOBJ);
    ret.push_back(Pair("isvalid", isValid));
    if (isValid)
    {
        CTxDestination dest = address.Get();
        string currentAddress = address.ToString();
        ret.push_back(Pair("address", currentAddress));

        CScript scriptPubKey = GetScriptForDestination(dest);
        ret.push_back(Pair("scriptPubKey", HexStr(scriptPubKey.begin(), scriptPubKey.end())));

#ifdef ENABLE_WALLET
        isminetype mine = pwalletMain ? IsMine(*pwalletMain, dest) : ISMINE_NO;
        ret.push_back(Pair("ismine", (mine & ISMINE_SPENDABLE) ? true : false));
        ret.push_back(Pair("iswatchonly", (mine & ISMINE_WATCH_ONLY) ? true: false));
        UniValue detail = boost::apply_visitor(DescribeAddressVisitor(), dest);
        ret.pushKVs(detail);
        if (pwalletMain && pwalletMain->mapAddressBook.count(dest))
            ret.push_back(Pair("account", pwalletMain->mapAddressBook[dest].name));
        CKeyID keyID;
        if (pwalletMain) {
            const auto& meta = pwalletMain->mapKeyMetadata;
            auto it = address.GetKeyID(keyID) ? meta.find(keyID) : meta.end();
            if (it == meta.end()) {
                it = meta.find(CScriptID(scriptPubKey));
            }
            if (it != meta.end()) {
                ret.push_back(Pair("timestamp", it->second.nCreateTime));
                if (!it->second.hdKeypath.empty()) {
                    ret.push_back(Pair("hdkeypath", it->second.hdKeypath));
                    ret.push_back(Pair("hdmasterkeyid", it->second.hdMasterKeyID.GetHex()));
                }
            }
        }
#endif
    }
    return ret;
}

/**
 * Used by addmultisigaddress / createmultisig:
 */
CScript _createmultisig_redeemScript(const UniValue& params)
{
    int nRequired = params[0].get_int();
    const UniValue& keys = params[1].get_array();

    // Gather public keys
    if (nRequired < 1)
        throw runtime_error("a multisignature address must require at least one key to redeem");
    if ((int)keys.size() < nRequired)
        throw runtime_error(
            strprintf("not enough keys supplied "
                      "(got %u keys, but need at least %d to redeem)", keys.size(), nRequired));
    if (keys.size() > 16)
        throw runtime_error("Number of addresses involved in the multisignature address creation > 16\nReduce the number");
    std::vector<CPubKey> pubkeys;
    pubkeys.resize(keys.size());
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        const std::string& ks = keys[i].get_str();
#ifdef ENABLE_WALLET
        // Case 1: ipchain address and we have full public key:
        CBitcoinAddress address(ks);
        if (pwalletMain && address.IsValid())
        {
            CKeyID keyID;
            if (!address.GetKeyID(keyID))
                throw runtime_error(
                    strprintf("%s does not refer to a key",ks));
            CPubKey vchPubKey;
            if (!pwalletMain->GetPubKey(keyID, vchPubKey))
                throw runtime_error(
                    strprintf("no full public key for address %s",ks));
            if (!vchPubKey.IsFullyValid())
                throw runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }

        // Case 2: hex public key
        else
#endif
        if (IsHex(ks))
        {
            CPubKey vchPubKey(ParseHex(ks));
            if (!vchPubKey.IsFullyValid())
                throw runtime_error(" Invalid public key: "+ks);
            pubkeys[i] = vchPubKey;
        }
        else
        {
            throw runtime_error(" Invalid public key: "+ks);
        }
    }
    CScript result = GetScriptForMultisig(nRequired, pubkeys);

    if (result.size() > MAX_SCRIPT_ELEMENT_SIZE)
        throw runtime_error(
                strprintf("redeemScript exceeds size limit: %d > %d", result.size(), MAX_SCRIPT_ELEMENT_SIZE));

    return result;
}

UniValue createmultisig(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() < 2 || request.params.size() > 2)
    {
        string msg = "createmultisig nrequired [\"key\",...]\n"
            "\nCreates a multi-signature address with n signature of m keys required.\n"
            "It returns a json object with the address and redeemScript.\n"

            "\nArguments:\n"
            "1. nrequired      (numeric, required) The number of required signatures out of the n keys or addresses.\n"
            "2. \"keys\"       (string, required) A json array of keys which are ipchain addresses or hex-encoded public keys\n"
            "     [\n"
            "       \"key\"    (string) ipchain address or hex-encoded public key\n"
            "       ,...\n"
            "     ]\n"

            "\nResult:\n"
            "{\n"
            "  \"address\":\"multisigaddress\",  (string) The value of the new multisig address.\n"
            "  \"redeemScript\":\"script\"       (string) The string value of the hex-encoded redemption script.\n"
            "}\n"

            "\nExamples:\n"
            "\nCreate a multisig address from 2 addresses\n"
            + HelpExampleCli("createmultisig", "2 \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("createmultisig", "2, \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"")
        ;
        throw runtime_error(msg);
    }

    // Construct using pay-to-script-hash:
    CScript inner = _createmultisig_redeemScript(request.params);
    CScriptID innerID(inner);
    CBitcoinAddress address(innerID);

    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("address", address.ToString()));
    result.push_back(Pair("redeemScript", HexStr(inner.begin(), inner.end())));

    return result;
}

UniValue verifymessage(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 3)
        throw runtime_error(
            "verifymessage \"address\" \"signature\" \"message\"\n"
            "\nVerify a signed message\n"
            "\nArguments:\n"
            "1. \"address\"         (string, required) The ipchain address to use for the signature.\n"
            "2. \"signature\"       (string, required) The signature provided by the signer in base 64 encoding (see signmessage).\n"
            "3. \"message\"         (string, required) The message that was signed.\n"
            "\nResult:\n"
            "true|false   (boolean) If the signature is verified or not.\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\", \"signature\", \"my message\"")
        );

    LOCK(cs_main);

    string strAddress  = request.params[0].get_str();
    string strSign     = request.params[1].get_str();
    string strMessage  = request.params[2].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
        return false;

    return (pubkey.GetID() == keyID);
}

UniValue signmessagewithprivkey(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 2)
        throw runtime_error(
            "signmessagewithprivkey \"privkey\" \"message\"\n"
            "\nSign a message with the private key of an address\n"
            "\nArguments:\n"
            "1. \"privkey\"         (string, required) The private key to sign the message with.\n"
            "2. \"message\"         (string, required) The message to create a signature of.\n"
            "\nResult:\n"
            "\"signature\"          (string) The signature of the message encoded in base 64\n"
            "\nExamples:\n"
            "\nCreate the signature\n"
            + HelpExampleCli("signmessagewithprivkey", "\"privkey\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XX\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("signmessagewithprivkey", "\"privkey\", \"my message\"")
        );

    string strPrivkey = request.params[0].get_str();
    string strMessage = request.params[1].get_str();

    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strPrivkey);
    if (!fGood)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");
    CKey key = vchSecret.GetKey();
    if (!key.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

UniValue setmocktime(const JSONRPCRequest& request)
{
    if (request.fHelp || request.params.size() != 1)
        throw runtime_error(
            "setmocktime timestamp\n"
            "\nSet the local time to given timestamp (-regtest only)\n"
            "\nArguments:\n"
            "1. timestamp  (integer, required) Unix seconds-since-epoch timestamp\n"
            "   Pass 0 to go back to using the system time."
        );

    if (!Params().MineBlocksOnDemand())
        throw runtime_error("setmocktime for regression testing (-regtest mode) only");

    // For now, don't change mocktime if we're in the middle of validation, as
    // this could have an effect on mempool time-based eviction, as well as
    // IsCurrentForFeeEstimation() and IsInitialBlockDownload().
    // TODO: figure out the right way to synchronize around mocktime, and
    // ensure all callsites of GetTime() are accessing this safely.
    LOCK(cs_main);

    RPCTypeCheck(request.params, boost::assign::list_of(UniValue::VNUM));
    SetMockTime(request.params[0].get_int64());

    return NullUniValue;
}

static UniValue RPCLockedMemoryInfo()
{
    LockedPool::Stats stats = LockedPoolManager::Instance().stats();
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("used", uint64_t(stats.used)));
    obj.push_back(Pair("free", uint64_t(stats.free)));
    obj.push_back(Pair("total", uint64_t(stats.total)));
    obj.push_back(Pair("locked", uint64_t(stats.locked)));
    obj.push_back(Pair("chunks_used", uint64_t(stats.chunks_used)));
    obj.push_back(Pair("chunks_free", uint64_t(stats.chunks_free)));
    return obj;
}

UniValue getmemoryinfo(const JSONRPCRequest& request)
{
    /* Please, avoid using the word "pool" here in the RPC interface or help,
     * as users will undoubtedly confuse it with the other "memory pool"
     */
    if (request.fHelp || request.params.size() != 0)
        throw runtime_error(
            "getmemoryinfo\n"
            "Returns an object containing information about memory usage.\n"
            "\nResult:\n"
            "{\n"
            "  \"locked\": {               (json object) Information about locked memory manager\n"
            "    \"used\": xxxxx,          (numeric) Number of bytes used\n"
            "    \"free\": xxxxx,          (numeric) Number of bytes available in current arenas\n"
            "    \"total\": xxxxxxx,       (numeric) Total number of bytes managed\n"
            "    \"locked\": xxxxxx,       (numeric) Amount of bytes that succeeded locking. If this number is smaller than total, locking pages failed at some point and key data could be swapped to disk.\n"
            "    \"chunks_used\": xxxxx,   (numeric) Number allocated chunks\n"
            "    \"chunks_free\": xxxxx,   (numeric) Number unused chunks\n"
            "  }\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getmemoryinfo", "")
            + HelpExampleRpc("getmemoryinfo", "")
        );
    UniValue obj(UniValue::VOBJ);
    obj.push_back(Pair("locked", RPCLockedMemoryInfo()));
    return obj;
}

UniValue echo(const JSONRPCRequest& request)
{
    if (request.fHelp)
        throw runtime_error(
            "echo|echojson \"message\" ...\n"
            "\nSimply echo back the input arguments. This command is for testing.\n"
            "\nThe difference between echo and echojson is that echojson has argument conversion enabled in the client-side table in"
            "ipchain-cli and the GUI. There is no server-side difference."
        );

    return request.params;
}
//add by xxy
UniValue getaddresstxids(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() <1 || request.params.size() >3)
		throw runtime_error(
		"getaddresstxids  \"address\"  (\"nCount\" \"hash\")\n"
		"\nReturns the txids for an address (requires addressindex to be enabled).\n"
		"\nArguments:\n"
		"{\n"
        "1.      \"address\"  (string) The base58check encoded address\n"
        "2.		\"nCount\"	(numeric)	The num how many txids you want to get\n"
        "3.		\"hash\"	(sting)	which txid where you want to select forward\n"
		"}\n"
		"\nResult:\n"
		"{\n"
		" \"txids[n]\"  (string) The txids of the address.\n"
		"}\n"
		"\nExamples:\n"
        + HelpExampleCli("getaddresstxids", "12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX")
        + HelpExampleRpc("getaddresstxids", "12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX")
		);
	if (!fAddressIndex)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No support!");
	}
	std::string straddress = request.params[0].get_str();
	if (straddress.length() != 36&&straddress.length() != 35)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invailed address!");
	}
	int nCount = NUM_OFDIS;
	if (request.params.size() >1)
		nCount = std::atoi(request.params[1].get_str().c_str());
	if (nCount < NUM_OFDIS)
		nCount = NUM_OFDIS; 
	std::string hash = "";
	if (request.params.size() >2)
		hash=request.params[2].get_str();

	std::vector<std::string> txids;
	txids.clear();

	if (hash == "")
	{
		if (!(pTxDB->Select(pTxDB->db, (char*)(straddress.c_str()), txids, nCount)))
		{
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this address has no txids");
		}
	}
	else{
		if (!(pTxDB->Select(pTxDB->db, (char*)(straddress.c_str()), txids, nCount, (char*)(hash.c_str()))))
		{
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this address has no txids");
		}
	}

	UniValue result(UniValue::VARR);
	for (int i = 0; i < txids.size(); i++)
		result.push_back(txids[i]);

	return result;
}

UniValue getaddressbalance(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() != 1)
		throw runtime_error(
        "getaddressbalance   \"address\"\n"
		"\nReturns the balance for an address (requires addressindex to be enabled).\n"
		"\nArguments:\n"
		"{\n"
        "1. \"address\"  (string) The base58check encoded address\n"
		"}\n"
		"\nResult:\n"
		"{\n"
		"  \"balance\"  (string) The current balance in satoshis\n"
		"  \"received\"  (string) The total number of satoshis received (including change)\n"
		"\nExamples:\n"
		"}\n"
        + HelpExampleCli("getaddressbalance", "address")
		);
	if (!fAddressIndex)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No support!");
	}
	std::string straddress = request.params[0].get_str();
	if (straddress.length() != 36&&straddress.length() != 35)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invailed address!");
	}
	CAmount balance = 0;
	CAmount received = 0;
	CAmount sended = 0;
	uint64_t txidnum = 0;
	if (!getAddressBalanceByTxlevel(straddress, balance, received, sended, txidnum))
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this address has no txids ");
	}
	UniValue result(UniValue::VOBJ);
	result.push_back(Pair("balance", ValueFromAmount(balance)));
	result.push_back(Pair("received", ValueFromAmount(received)));
	result.push_back(Pair("sended", ValueFromAmount(sended)));
	result.push_back(Pair("txidnum", txidnum));

	return result;
}

UniValue getipchashtxids(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() != 1)
		throw runtime_error(
		"getipchashtxids  \"ipchash\"\n"
		"\nReturns the txids for an ipchash (requires ipchash to be enabled).\n"
		"\nArguments:\n"
		"{\n"
        "1     \"ipchash\"  (string) The uint128 encoded ipchash\n"
		"}\n"
		"\nResult:\n"
		"{\n"
		" \"txids[n]\"  (string) The txids of the ipchash.\n"
		"}\n"
		"\nExamples:\n"
		+ HelpExampleCli("getipchashtxids", "'{\"ipchash\": [\"12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX\"]}'")
		+ HelpExampleRpc("getipchashtxids", "{\"ipchash\": [\"12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX\"]}")
		);

	if (!fAddressIndex)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No support!");
	}
	std::string ipchash = request.params[0].get_str();
	if (ipchash.length() != 32)
	{
		throw JSONRPCError(RPC_TYPE_ERROR, "this ipchash length is Wrongful ");
	}
	uint128 ipcHash;
	ipcHash.SetHex(ipchash);
	if (ipcHash.IsNull())
	{
		throw JSONRPCError(RPC_TYPE_ERROR, "this ipchash was Wrongful ");
	}

	std::vector<std::string> txids;
	txids.clear();
	if (!pTxDB->Select(pTxDB->db, (char*)(ipchash.c_str()),txids,0))
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this ipchash has no txids");
	}
	UniValue result(UniValue::VARR);
	for (int i = 0; i < txids.size(); i++)
		result.push_back(txids[i]);
	return result;
}

UniValue gettokensymboltxids(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() <1 || request.params.size() >3)
		throw runtime_error(
        "gettokensymboltxids \"tokensymbol\"  (\"nCount\"  \"txid\")\n"
		"\nReturns the txids for an tokensymbol (requires tokensymbol to be enabled).\n"
		"\nArguments:\n"
		"{\n"
        "1.    \"tokensymbol\"  (string) The tokensymbol encoded tokenTx\n"
        "2.    \"nCount\"	(numeric)	The num how many txids you want to get\n"
        "3.    \"txid\"	(sting)	which txid where you want to select forward\n"
		"}\n"
		"\nResult:\n"
		"{\n"
		" \"txids[n]\"  (string) The txids of the tokensymbol.\n"
		"}\n"
		"\nExamples:\n"
		+ HelpExampleCli("gettokensymboltxids", "'{\"tokensymbol\": [\"1rJX\"]}'")
		+ HelpExampleRpc("gettokensymboltxids", "{\"tokensymbol\": [\"1rJX\"]}")
		);
	if (!fAddressIndex)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No support!");
	}
	std::string tokensymbol = request.params[0].get_str();
	int nCount = NUM_OFDIS;
	if (request.params.size() > 1)
		nCount = std::atoi(request.params[1].get_str().c_str());
	if (nCount < NUM_OFDIS)
		nCount = NUM_OFDIS; 
	std::string hash = "";
	if (request.params.size() >2)
		hash = request.params[2].get_str();

	std::vector<std::string> txids;
	txids.clear();
	if (hash == "")
	{
		if (!(pTxDB->Select(pTxDB->db, (char*)(tokensymbol.c_str()), txids,nCount)))
		{
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this tokensymbol has no txids");
		}
	}
	else{
		if (!(pTxDB->Select(pTxDB->db, (char*)(tokensymbol.c_str()), txids, nCount, (char*)(hash.c_str()))))
		{
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this tokensymbol has no txids");
		}
	}
	
	UniValue result(UniValue::VARR);
	for (int i = 0; i < txids.size(); i++)
		result.push_back(txids[i]);

	return result;
}
UniValue gettokenlabelbysymbol(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() <1 || request.params.size() >1)
		throw runtime_error(
		"gettokenlabelbysymbol \"tokensymbol\"  \n"
		"\nReturns the tokeninfo for an tokensymbol (requires tokensymbol to be enabled).\n"
		"\nArguments:\n"
		"{\n"
		"      \"tokensymbol\"  (string) The tokensymbol encoded tokenTx\n"
		"}\n"
		"\nResult:\n"
		"{\n"
		" \"tokenSymbol\"  (string) tokensymbol.\n"
		" \"totalCount\"   (uint64_t) totalCount.\n"
		" \"tokenhash\"    (uint128) MD5 tokenhash.\n"
		" \"label\"        (uint8_t[17]) label.\n"
		" \"regtime\"      (uint32_t) issueDate.\n"
		"}\n"
		"\nExamples:\n"
		+ HelpExampleCli("gettokenlabelbysymbol", "'{\"tokensymbol\": [\"1rJX\"]}'")
		+ HelpExampleRpc("gettokenlabelbysymbol", "{\"tokensymbol\": [\"1rJX\"]}")
		);
	std::string tokensymbol = request.params[0].get_str();
	UniValue result(UniValue::VOBJ);
	if (!tokenDataMap.count(tokensymbol))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this tokensymbol has no TokenReg");
	
	result.push_back(Pair("tokenSymbol", tokenDataMap[tokensymbol].getTokenSymbol()));
	result.push_back(Pair("totalCount", ValueFromTCoins(tokenDataMap[tokensymbol].totalCount, (int)tokenDataMap[tokensymbol].accuracy)));
	result.push_back(Pair("accuracy", tokenDataMap[tokensymbol].accuracy));
	result.push_back(Pair("tokenhash", tokenDataMap[tokensymbol].hash.GetHex()));
	result.push_back(Pair("label", tokenDataMap[tokensymbol].getTokenLabel()));
	result.push_back(Pair("regtime", tokenDataMap[tokensymbol].issueDate));
	
	
	return result;
}

UniValue gettokenbalancebyaddress(const JSONRPCRequest& request)
{
	if (request.fHelp || request.params.size() != 2)
		throw runtime_error(
		"gettokenbalancebyaddress   \"addresses\" \"tokensymbol\"\n"
		"\nReturns the balance for an address (requires addressindex to be enabled).\n"
		"\nArguments:\n"
		"{\n"
        "1.  \"address\"  (string) The base58check encoded address\n"
        "2.  \"tokensymbol\"  (string) The tokensymbol encoded tokenTx\n"
		"}\n"
		"\nResult:\n"
		"{\n"
		"  \"tokenbalance\"  (string) The current balance in satoshis\n"
		"}\n"
        + HelpExampleCli("gettokenbalancebyaddress", "\"addresses\" \"tokensymbol\"")
		);
	if (!fAddressIndex)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No support!");
	}
	std::string straddress = request.params[0].get_str();
	if (straddress.length() != 36)
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invailed address!");
	}
	std::string tokensymbol = request.params[1].get_str();
	if (!tokenDataMap.count(tokensymbol))
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this tokensymbol has no TokenReg");
	CAmount balance = 0;
	CAmount received = 0;
	CAmount sended = 0;
	uint64_t txidnum = 0;
	int nCount = NUM_OFDIS;
	if (!getTokenBalanceByAddress(straddress, tokensymbol,balance,received, sended, txidnum))
	{
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "this address has no txids ");
	}
	UniValue result(UniValue::VOBJ);
	result.push_back(Pair("address", straddress));
	result.push_back(Pair("tokensymbol", tokensymbol));
	result.push_back(Pair("tokenbalance", ValueFromTCoins(balance, (int)tokenDataMap[tokensymbol].accuracy)));
	return result;
}
//end
static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         okSafeMode
  //  --------------------- ------------------------  -----------------------  ----------
    { "control",            "getinfo",                &getinfo,                true,  {} }, /* uses wallet if enabled */
	{ "control",			"getIPCversion",		  &getIPCversion,		   true, {} }, /* uses wallet if enabled */
    { "control",            "getmemoryinfo",          &getmemoryinfo,          true,  {} },
    { "util",               "validateaddress",        &validateaddress,        true,  {"address"} }, /* uses wallet if enabled */
//    { "util",               "createmultisig",         &createmultisig,         true,  {"nrequired","keys"} },
    { "util",               "verifymessage",          &verifymessage,          true,  {"address","signature","message"} },
    { "util",               "signmessagewithprivkey", &signmessagewithprivkey, true,  {"privkey","message"} },
	{ "util",				"getaddresstxids",		  &getaddresstxids,		   true,  { "address" "nCount" "txid" } },
	{ "util",				"getaddressbalance",	  &getaddressbalance,		true,  { "address" } },
	{ "util",				"gettokenbalancebyaddress", &gettokenbalancebyaddress,  true,  { "address" "tokensymbol"} },
	{ "util",				"getipchashtxids",		  &getipchashtxids,			true,  { "ipchash" } },
	{ "util",				"gettokensymboltxids",	  &gettokensymboltxids,		true,  { "tokensymbol" "nCount" "txid"} },
	{ "util",				"gettokenlabelbysymbol",  &gettokenlabelbysymbol,	true, { "tokensymbol" } },
    /* Not shown in help */
//    { "hidden",             "setmocktime",            &setmocktime,            true,  {"timestamp"}},
    { "hidden",             "echo",                   &echo,                   true,	{"arg0","arg1","arg2","arg3","arg4","arg5","arg6","arg7","arg8","arg9"}},
    { "hidden",             "echojson",               &echo,                  true,  {"arg0","arg1","arg2","arg3","arg4","arg5","arg6","arg7","arg8","arg9"}},
};

void RegisterMiscRPCCommands(CRPCTable &t)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        t.appendCommand(commands[vcidx].name, &commands[vcidx]);
}

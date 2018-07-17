#include "unionaccountcreate.h"
#include "ui_unionaccountcreate.h"
#include "log/log.h"
#include "recentrequeststablemodel.h"
#include <QSortFilterProxyModel>
#include "addresstablemodel.h"
#ifdef ENABLE_WALLET
#include "walletmodel.h"
#endif
unionaccountcreate::unionaccountcreate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::unionaccountcreate)
{
    ui->setupUi(this);
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    ui->comboBox->addItems(QStringList()<<"2"<<"3"<<"4"<<"5");
    ui->comboBox_2->addItems(QStringList()<<"2"<<"3"<<"4"<<"5");
    connect(ui->comboBox, SIGNAL(activated(int)), this, SLOT(coinUpdate(int)));

    //
    ui->lineEdit_key1->setVisible(true);
    ui->label_u1->setVisible(true);

    ui->lineEdit_key2->setVisible(false);
    ui->label_u2->setVisible(false);

    ui->lineEdit_key3->setVisible(false);
    ui->label_u3->setVisible(false);

    ui->lineEdit_key4->setVisible(false);
    ui->label_u4->setVisible(false);



    ui->line_4->setVisible(true);

    ui->lineEdit_key1->setText("");
    ui->lineEdit_key2->setText("");
    ui->lineEdit_key3->setText("");
    ui->lineEdit_key4->setText("");

    QRegExp regx("^\\S+$");
    QValidator *validator = new QRegExpValidator(regx, ui->lineEdit_name );
    ui->lineEdit_name->setValidator( validator );




}

void unionaccountcreate::coinUpdate(int idx)
{
    switch(ui->comboBox->currentIndex()+1)
    {
    /*
    case 1:
    {
        ui->lineEdit_key1->setVisible(false);
        ui->label_u1->setVisible(false);

        ui->lineEdit_key2->setVisible(false);
        ui->label_u2->setVisible(false);

        ui->lineEdit_key3->setVisible(false);
        ui->label_u3->setVisible(false);

        ui->lineEdit_key4->setVisible(false);
        ui->label_u4->setVisible(false);

        ui->line_4->setVisible(false);

        ui->lineEdit_key1->setText("");
        ui->lineEdit_key2->setText("");
        ui->lineEdit_key3->setText("");
        ui->lineEdit_key4->setText("");
    }
        break;
        */
    case 1:
    {


        ui->lineEdit_key1->setVisible(true);
        ui->label_u1->setVisible(true);

        ui->lineEdit_key2->setVisible(false);
        ui->label_u2->setVisible(false);

        ui->lineEdit_key3->setVisible(false);
        ui->label_u3->setVisible(false);

        ui->lineEdit_key4->setVisible(false);
        ui->label_u4->setVisible(false);



        ui->line_4->setVisible(true);

        ui->lineEdit_key1->setText("");
        ui->lineEdit_key2->setText("");
        ui->lineEdit_key3->setText("");
        ui->lineEdit_key4->setText("");


    }
        break;
    case 2:
    {
        ui->lineEdit_key1->setVisible(true);
        ui->label_u1->setVisible(true);

        ui->lineEdit_key2->setVisible(true);
        ui->label_u2->setVisible(true);

        ui->lineEdit_key3->setVisible(false);
        ui->label_u3->setVisible(false);

        ui->lineEdit_key4->setVisible(false);
        ui->label_u4->setVisible(false);

        ui->line_4->setVisible(true);

        ui->lineEdit_key1->setText("");
        ui->lineEdit_key2->setText("");
        ui->lineEdit_key3->setText("");
        ui->lineEdit_key4->setText("");


    }
        break;
    case 3:
    {
        ui->lineEdit_key1->setVisible(true);
        ui->label_u1->setVisible(true);

        ui->lineEdit_key2->setVisible(true);
        ui->label_u2->setVisible(true);

        ui->lineEdit_key3->setVisible(true);
        ui->label_u3->setVisible(true);

        ui->lineEdit_key4->setVisible(false);
        ui->label_u4->setVisible(false);

        ui->line_4->setVisible(true);

        ui->lineEdit_key1->setText("");
        ui->lineEdit_key2->setText("");
        ui->lineEdit_key3->setText("");
        ui->lineEdit_key4->setText("");
    }
        break;
    case 4:
    {
        ui->lineEdit_key1->setVisible(true);
        ui->label_u1->setVisible(true);

        ui->lineEdit_key2->setVisible(true);
        ui->label_u2->setVisible(true);

        ui->lineEdit_key3->setVisible(true);
        ui->label_u3->setVisible(true);

        ui->lineEdit_key4->setVisible(true);
        ui->label_u4->setVisible(true);

        ui->line_4->setVisible(true);

        ui->lineEdit_key1->setText("");
        ui->lineEdit_key2->setText("");
        ui->lineEdit_key3->setText("");
        ui->lineEdit_key4->setText("");
    }
        break;
    default:
    {
    }
    }
}

unionaccountcreate::~unionaccountcreate()
{
    delete ui;
}
void unionaccountcreate::setModel(WalletModel *_model)
{
    this->model = _model;

}
void unionaccountcreate::setInit()
{

    ui->comboBox->tr("2");
    ui->comboBox_2->setCurrentIndex(0);
    ui->comboBox->setCurrentIndex(0);
    ui->label_error->setText("");
    ui->lineEdit_name->setText("");
    ui->lineEdit_key->setText("");
    ui->lineEdit_key1->setText("");
    ui->lineEdit_key2->setText("");
    ui->lineEdit_key3->setText("");
    ui->lineEdit_key4->setText("");

    //

    ui->lineEdit_key1->setVisible(true);
    ui->label_u1->setVisible(true);

    ui->lineEdit_key2->setVisible(false);
    ui->label_u2->setVisible(false);

    ui->lineEdit_key3->setVisible(false);
    ui->label_u3->setVisible(false);

    ui->lineEdit_key4->setVisible(false);
    ui->label_u4->setVisible(false);



    ui->line_4->setVisible(true);

    ui->lineEdit_key1->setText("");
    ui->lineEdit_key2->setText("");
    ui->lineEdit_key3->setText("");
    ui->lineEdit_key4->setText("");

}
void unionaccountcreate::on_createBtn_pressed()
{

    ui->label_error->clear();
    ui->label_error->setVisible(false);
    ui->label_error->setText("");
    if(ui->lineEdit_name->text() == "" ||  ui->lineEdit_key->text() == "" )
    {
        ui->label_error->setVisible(true);
        ui->label_error->setText(tr("input info"));
        return;
    }

    ui->label_error->setVisible(true);
    ui->label_error->setText("");
    std::string add1_ = ui->lineEdit_key->text().toStdString();
    std::string add2_ = ui->lineEdit_key1->text().toStdString();
    std::string add3_ = ui->lineEdit_key2->text().toStdString();
    std::string add4_ = ui->lineEdit_key3->text().toStdString();
    std::string add5_ = ui->lineEdit_key4->text().toStdString();
    std::vector<std::string> vec_add;

    int num = ui->comboBox->currentIndex()+1;


    QString my_num = ui->comboBox->currentText();

    int conf_num = ui->comboBox_2->currentIndex()+2;

    //conf_num =1 ;

    LOG_WRITE(LOG_INFO,"my_num,conf_num",my_num.toStdString().c_str(),QString::number(conf_num).toStdString().c_str());
    if("2" == my_num && ui->lineEdit_key1->text() == "")
    {
        ui->label_error->setText(tr("input info"));
        return;
    }

    if("3" == my_num && (ui->lineEdit_key1->text() == "" || ui->lineEdit_key2->text() == ""))
    {
        ui->label_error->setText(tr("input info"));
        return;
    }
    if("4" == my_num && (ui->lineEdit_key1->text() == "" || ui->lineEdit_key2->text() == "" || ui->lineEdit_key3->text() == ""))
    {
        ui->label_error->setText(tr("input info"));
        return;
    }
    if("5" == my_num && (ui->lineEdit_key1->text() == "" || ui->lineEdit_key2->text() == "" || ui->lineEdit_key3->text() == "" || ui->lineEdit_key4->text() == ""))
    {
        ui->label_error->setText(tr("input info"));
        return;
    }
    int mypknum = 0;
    if(add1_ !="")
    {
        vec_add.push_back(add1_);
        if(model->isMyPk(add1_)){
            mypknum++;
        }
    }
    if(add2_ !="")    { vec_add.push_back(add2_); if(model->isMyPk(add2_)){LOG_WRITE(LOG_INFO,"num2");mypknum++;}}
    if(add3_ !="")    { vec_add.push_back(add3_); if(model->isMyPk(add3_)){LOG_WRITE(LOG_INFO,"num3");mypknum++;}}
    if(add4_ !="")    { vec_add.push_back(add4_); if(model->isMyPk(add4_)){LOG_WRITE(LOG_INFO,"num4");mypknum++;}}
    if(add5_ !="")    { vec_add.push_back(add5_); if(model->isMyPk(add5_)){LOG_WRITE(LOG_INFO,"num5");mypknum++;}}
    if(mypknum!=1){
        LOG_WRITE(LOG_INFO,"mypknum ",QString::number(mypknum).toStdString().c_str());

        ui->label_error->setText(tr("Please make sure that there is  only one of your own public key."));
        return;
    }

   // for(int i = 0; i< vec_add.size(); i++)
   // {
   //     std::string d1 = vec_add.at(i);
  //  }
    int ncount;
    for(int i = 0; i< vec_add.size(); i++)
    {
        ncount = count(vec_add.begin(),vec_add.end(),vec_add.at(i));
        if(ncount>1)
        {
            ui->label_error->setText(tr("Publickey repetition"));
            return;
        }
    }
    LOG_WRITE(LOG_INFO,"CreateUnionAddress");
    std::string union_script,union_add,m_failreason;
    if(model->CreateUnionAddress(conf_num,vec_add,union_script,union_add,m_failreason))
    {

        ui->label_error->setText("");
        if(model->joinunionaccount(ui->lineEdit_name->text().toStdString(),m_failreason,union_script))
        {
            ui->label_error->setText("");
            LOG_WRITE(LOG_INFO,"joinunionaccount script + add",union_script.c_str(),union_add.c_str());
            Q_EMIT refreshunionaccount();
            Q_EMIT opensuccesscreatePage(QString::fromStdString(union_script),QString::fromStdString(union_add));
        }
        else
        {
            if("script is not p2sh script!" ==m_failreason )
            {
                ui->label_error->setText(tr("script is not p2sh script!"));
            }
            else if("AddMultiAddress failed!" ==m_failreason)
            {
                ui->label_error->setText(tr("AddMultiAddress failed!"));
            }
            else if("MultiAdd is not yours" == m_failreason)
            {
                ui->label_error->setText(tr("MultiAdd is not yours"));
            }
            else if("Address duplication!"== m_failreason)
            {
                ui->label_error->setText(tr("Address duplication"));
            }
            else if("Password error."== m_failreason)
            {
                ui->label_error->setText(tr("Password error."));
            }
            else
            {
                LOG_WRITE(LOG_INFO,"JOIN FAIL",m_failreason.c_str());
                ui->label_error->setText(tr("join error")+" "+QString::fromStdString(m_failreason));
            }

        }
    }
    else
    {

        if("nRequired or strPubkeys size is valid!" ==m_failreason )
        {
            ui->label_error->setText(tr("nRequired or strPubkeys size is valid!"));
        }
        else if("Pubkey is valid!" ==m_failreason)
        {
            ui->label_error->setText(tr("Pubkey is valid!"));
        }
        else if("CScript size too large!" ==m_failreason)
        {
            ui->label_error->setText(tr("CScript size too large!"));
        }
        else if("script is not p2sh script!" ==m_failreason )
        {
            ui->label_error->setText(tr("script is not p2sh script!"));
        }
        else if("AddMultiAddress failed!" ==m_failreason)
        {
            ui->label_error->setText(tr("AddMultiAddress failed!"));
        }

        else
        {
            LOG_WRITE(LOG_INFO,"CREATE FAIL",m_failreason.c_str());
            ui->label_error->setText(tr("create error")+" "+QString::fromStdString(m_failreason));
        }
    }
}
void unionaccountcreate::showEvent(QShowEvent *event)
{
    ui->label_error->setText("");
    ui->lineEdit_name->setText("");
    ui->lineEdit_key->setText("");
    ui->lineEdit_key1->setText("");
    ui->lineEdit_key2->setText("");
    ui->lineEdit_key3->setText("");
    ui->lineEdit_key4->setText("");

    //

    ui->lineEdit_key1->setVisible(true);
    ui->label_u1->setVisible(true);

    ui->lineEdit_key2->setVisible(false);
    ui->label_u2->setVisible(false);

    ui->lineEdit_key3->setVisible(false);
    ui->label_u3->setVisible(false);

    ui->lineEdit_key4->setVisible(false);
    ui->label_u4->setVisible(false);



    ui->line_4->setVisible(true);

    ui->lineEdit_key1->setText("");
    ui->lineEdit_key2->setText("");
    ui->lineEdit_key3->setText("");
    ui->lineEdit_key4->setText("");
}

void unionaccountcreate::on_btn_genkey_pressed()
{
    ui->label_error->setText("");
    if(model && model->getOptionsModel())
    {
        model->getRecentRequestsTableModel()->sort(RecentRequestsTableModel::Date, Qt::DescendingOrder);
        QSortFilterProxyModel * proxyModel;
        proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(model->getAddressTableModel());
        proxyModel->setDynamicSortFilter(true);
        proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxyModel->setFilterRole(AddressTableModel::TypeRole);
        proxyModel->setFilterFixedString(AddressTableModel::Receive);

        QModelIndex buttomindex=  proxyModel->index(0,1, QModelIndex());
        QVariant bumvar= buttomindex.data();
        QString str = bumvar.toString();;
        std::string m_key;
        std::string m_failreason;
        std::string m_str = str.toStdString();





        if(model->addtopubkey(m_str,m_key,m_failreason))
        {
            LOG_WRITE(LOG_INFO,"GENKEY-CREATE",m_key.c_str());
            ui->lineEdit_key->setText(QString::fromStdString(m_key));
        }
        else
        {

            if("address is valid!" ==m_failreason )
            {
                ui->label_error->setText(tr("address is valid!"));
            }
            else if("address can't be Script!" ==m_failreason)
            {
                ui->label_error->setText(tr("address can't be Script!"));
            }
            else if("GetPubKey faild!" ==m_failreason)
            {
                ui->label_error->setText(tr("GetPubKey faild!"));
            }
            else
            {
                ui->label_error->setText(tr("GenKey faild!"));
            }
        }





    }
}

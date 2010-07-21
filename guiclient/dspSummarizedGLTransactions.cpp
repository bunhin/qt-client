/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedGLTransactions.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "glTransactionDetail.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "invoice.h"
#include "purchaseOrder.h"
#include "mqlutil.h"

dspSummarizedGLTransactions::dspSummarizedGLTransactions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedSource, SIGNAL(toggled(bool)), _source, SLOT(setEnabled(bool)));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _gltrans->setRootIsDecorated(TRUE);
  _gltrans->addColumn(tr("Account #"),         150,               Qt::AlignCenter, true,  "account" );
  _gltrans->addColumn(tr("Date"),              _dateColumn,       Qt::AlignCenter, true,  "gltrans_date" );
  _gltrans->addColumn(tr("Description/Notes"), -1,                Qt::AlignLeft,   true,  "descrip_notes"   );
  _gltrans->addColumn(tr("Src."),              _whsColumn,        Qt::AlignCenter, true,  "gltrans_source" );
  _gltrans->addColumn(tr("Doc. Type"),         _docTypeColumn,    Qt::AlignCenter, true,  "gltrans_doctype" );
  _gltrans->addColumn(tr("Doc. #"),            _orderColumn,      Qt::AlignCenter, true,  "gltrans_docnumber" );
  _gltrans->addColumn(tr("Debit"),             _bigMoneyColumn,   Qt::AlignRight,  true,  "debit"  );
  _gltrans->addColumn(tr("Credit"),            _bigMoneyColumn,   Qt::AlignRight,  true,  "credit"  );
  _gltrans->addColumn( tr("Username"),         _userColumn,       Qt::AlignLeft,   true,  "gltrans_username" );
}

dspSummarizedGLTransactions::~dspSummarizedGLTransactions()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedGLTransactions::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedGLTransactions::sPopulateMenu(QMenu * menuThis)
{
  if(_gltrans->altId() == -1)
    return;

  menuThis->addAction(tr("View..."), this, SLOT(sViewTrans()));

  QTreeWidgetItem * item = _gltrans->currentItem();
  if(0 == item)
    return;

  if(item->text(4) == "VO")
    menuThis->addAction(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->text(4) == "IN")
    menuThis->addAction(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->text(4) == "PO")
    menuThis->addAction(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
}


void dspSummarizedGLTransactions::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);

  if (_unpostedTransactions->isChecked())
    params.append("unpostedTransactions");
  else if (_postedTransactions->isChecked())
    params.append("postedTransactions");

  if (_selectedSource->isChecked())
    params.append("source", _source->currentText());

  params.append("showUsernames");

  orReport report("SummarizedGLTransactions", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedGLTransactions::sFillList()
{
  _gltrans->clear();

  ParameterList params;
  if ( ! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("summarizedGLTransactions", "detail");
  q = mql.toQuery(params);

  if (q.first())
  {
    _gltrans->populate(q, true);
  }
}

bool dspSummarizedGLTransactions::setParams(ParameterList & params)
{
  if (_dates->allValid())
  {
    params.append("startDate", _dates->startDate());
    params.append("endDate", _dates->endDate());
  }

  if (_selectedSource->isChecked())
    params.append("source", _source->currentText());

  if (_unpostedTransactions->isChecked())
    params.append("unpostedTransactions");
  else if (_postedTransactions->isChecked())
    params.append("postedTransactions");

  return true;
}

void dspSummarizedGLTransactions::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", _gltrans->altId());

  glTransactionDetail newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspSummarizedGLTransactions::sViewDocument()
{
  QTreeWidgetItem * item = _gltrans->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->text(4) == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc "
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->text(5));
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");

    if(q.value("vohead_misc").toBool())
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
  else if(item->text(4) == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->text(5));
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->text(4) == "PO")
  {
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:pohead_number)");
    q.bindValue(":pohead_number", item->text(5));
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

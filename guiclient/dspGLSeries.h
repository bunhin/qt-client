/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPGLSERIES_H
#define DSPGLSERIES_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspGLSeries.h"

class dspGLSeries : public XWidget, public Ui::dspGLSeries
{
    Q_OBJECT

public:
    dspGLSeries(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspGLSeries();

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sPopulateMenu( QMenu * );
    virtual void sPrint();
    virtual void sFillList();
    virtual void sReverse();

protected slots:
    virtual void languageChange();

};

#endif // DSPGLSERIES_H
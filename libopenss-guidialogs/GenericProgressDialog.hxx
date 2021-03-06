////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
// Copyright (c) 2006-2013 Krell Institute. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////
  

#ifndef GENERICPROGRESSDIALOG_H
#define GENERICPROGRESSDIALOG_H

#include <stddef.h>
#include <qvariant.h>
#include <qdialog.h>
#include <qslider.h>

#include "openspeedshop.hxx"

class QVBoxLayout;
class QHBoxLayout;
class QFrame;
class QLabel;

class GenericProgressDialog : public QDialog
{
    Q_OBJECT

public:
    GenericProgressDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GenericProgressDialog();

    QLabel* infoLabel;
    QSlider *qs;
protected:
    QVBoxLayout* GenericProgressDialogLayout;

protected slots:
    virtual void languageChange();

public slots:
};

#endif // GENERICPROGRESSDIALOG_H

#pragma once
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 40
#define BOOST_THREAD_VERSION 3
#include "targetver.h"
#include <tchar.h>

// Wt
#pragma warning(push)
#pragma warning(disable:4800 4251 4275)
#include <Wt\WServer.h>
#include <Wt\WApplication.h>
#include <Wt\WBreak.h>
#include <Wt\WContainerWidget.h>
#include <Wt\WLineEdit.h>
#include <Wt\WPushButton.h>
#include <Wt\WText.h>
#include <Wt\WTextArea.h>
#include <Wt\WTextEdit.h>
#include <Wt\WMenu.h>
#include <Wt\WStackedWidget.h>
#include <Wt\WBootstrapTheme.h>
#include <Wt\WNavigationBar.h>
#include <Wt\WPopupMenu.h>
#include <Wt\WMessageBox.h>
#include <Wt\WHBoxLayout.h>
#include <Wt\WVBoxLayout.h>
#include <Wt\WPushButton.h>
#include <Wt\WTemplate.h>
#include <Wt\WTree.h>
#include <Wt\WTreeTable.h>
#include <Wt\WTreeTableNode.h>
#include <Wt\WLabel.h>
#include <Wt\WPanel.h>
#include <Wt\WGroupBox.h>
#include <Wt\WTreeView.h>
#include <Wt\WTableView.h>
#include <Wt\WStandardItem.h>
#include <Wt\WStandardItemModel.h>
#include <Wt\WAbstractItemModel.h>
#include <Wt\WAbstractTableModel.h>
#include <Wt\WEvent.h>
#include <Wt\WTabWidget.h>
#include <Wt\WTable.h>
#include <Wt\WTableCell.h>
#include <Wt\WComboBox.h>
#include <Wt\WToolBar.h>
#include <Wt\WAnchor.h>
#include <Wt\WItemDelegate.h>
#include <Wt\WSpinBox.h>
#include <Wt\WIntValidator.h>
#include <Wt\WImage.h>
#include <Wt\WFileUpload.h>
#include <Wt\WProgressBar.h>
#include <Wt\WDate.h>
#include <Wt\WDateEdit.h>
#include <Wt\WTimeEdit.h>
#include <Wt\Chart\WPieChart.h>
#include <Wt\WRadioButton.h>
#include <Wt\WButtonGroup.h>
#include <Wt\WTimer.h>
#include <Wt\WLoadingIndicator.h>
#include <Wt\Chart\WStandardPalette.h>
#include <Wt\Http\Request.h>
#include <Wt\Http\Response.h>
#include <Wt\WResource.h>

#include "WtTools.h"
#pragma warning(pop)

// Core
#include "TRCore\Core.h"
#include "TRCore\DirectClient.h"
#include "TRCore\DateTime.h"

// STLTools
#include "StlTools\stl_tools.h"

// STL
#include <memory>
#include <string>
#include <queue>
#include <stack>
#include <iomanip>

// BOOST
#include <boost\range.hpp>
#include <boost\range\adaptor\map.hpp>
#include <boost\range\adaptor\filtered.hpp>
#include <boost\range\adaptor\reversed.hpp>
#include <boost\variant\polymorphic_get.hpp>
#include <boost\format.hpp>
#include <boost\regex.hpp>
#include <boost\algorithm\cxx11\copy_if.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\lexical_cast.hpp>
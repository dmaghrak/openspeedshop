////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
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


#include "pcStatsPanel.hxx"   // Change this to your new class header file name
#include "StatsPanelBase.hxx"   // Change this to your new class header file name
#include "PanelContainer.hxx"   // Do not remove
#include "plugin_entry_point.hxx"   // Do not remove

#include <qvaluelist.h>
class MetricHeaderInfo;
typedef QValueList<MetricHeaderInfo *> MetricHeaderInfoList;


#include "SPListView.hxx"   // Change this to your new class header file name
#include "SPListViewItem.hxx"   // Change this to your new class header file name
#include "UpdateObject.hxx"
#include "SourceObject.hxx"
#include "PreferencesChangedObject.hxx"

// #include "preference_plugin_info.hxx" // Do not remove

#include "MetricInfo.hxx" // dummy data only...
// This is only hear for the debugging tables....
static char *color_name_table[10] =
  { "red", "orange", "yellow", "skyblue", "green" };


#include "ToolAPI.hxx"
using namespace OpenSpeedShop::Framework;


pcStatsPanel::pcStatsPanel(PanelContainer *pc, const char *n, void *argument) : StatsPanelBase(pc, n, argument)
{
//  printf("pcStatsPanel() entered\n");
  setCaption("pcStatsPanel");
}


/*! The only thing that needs to be cleaned is anything allocated in this
    class.  By default that is nothing.
 */
pcStatsPanel::~pcStatsPanel()
{
  // Delete anything you new'd from the constructor.
}

void
pcStatsPanel::languageChange()
{
  // Set language specific information here.
}


/*! When a message has been sent (from anyone) and the message broker is
    notifying panels that they may want to know about the message, this is the
    function the broker notifies.   The listener then needs to determine
    if it wants to handle the message.
    \param msg is the incoming message.
    \return 0 means you didn't do anything with the message.
    \return 1 means you handled the message.
 */
int 
pcStatsPanel::listener(void *msg)
{
//  printf("pcStatsPanel::listener() requested.\n");
//  StatsPanelBase::listener(msg);


  PreferencesChangedObject *pco = NULL;

// BUG - BIG TIME KLUDGE.   This should have a message type.
  MessageObject *msgObject = (MessageObject *)msg;
  if(  msgObject->msgType  == "UpdateExperimentDataObject" )
  {
    UpdateObject *msg = (UpdateObject *)msgObject;
msg->print();
#ifdef PRINT_DEBUG
PrintView(msg->expID);
#endif // PRINT_DEBUG
    updateStatsPanelBaseData(msg->fw_expr, msg->expID, msg->experiment_name);
    if( msg->raiseFLAG )
    {
      getPanelContainer()->raisePanel((Panel *)this);
    }
  } else if( msgObject->msgType == "PreferencesChangedObject" )
  {
//    printf("StatsPanelBase:  The preferences changed.\n");
    pco = (PreferencesChangedObject *)msgObject;
    preferencesChanged();
  }

  return 0;  // 0 means, did not want this message and did not act on anything.
}


/*! Create the context senstive menu for the report. */
bool
pcStatsPanel::createPopupMenu( QPopupMenu* contextMenu, const QPoint &pos )
{
//  printf ("pcStatsPanel: Popup the context sensitive menu here.... can you augment it with the default popupmenu?\n");
  
  QPopupMenu *panelMenu = new QPopupMenu(this);
  panelMenu->setCaption("Panel Menu");
  contextMenu->insertItem("&Panel Menu", panelMenu, CTRL+Key_C);
  panelMenu->insertSeparator();
  menu(panelMenu);

  if( lv->selectedItem() )
  {
  //  contextMenu->insertItem("Tell Me MORE about %d!!!", this, SLOT(details()), CTRL+Key_1 );
    contextMenu->insertItem("Go to source location...", this, SLOT(gotoSource()), CTRL+Key_1 );
    return( TRUE );
  }
  
  return( FALSE );
}

/*! Go to source menu item was selected. */
void
pcStatsPanel::gotoSource()
{
//  printf("gotoSource() menu selected.\n");
}

void
pcStatsPanel::itemSelected(QListViewItem *item)
{
//  printf("pcStatsPanel::itemSelected(clicked) entered\n");

  if( item )
  {
//    printf("  item->depth()=%d\n", item->depth() );

    SPListViewItem *nitem = (SPListViewItem *)item;
    while( nitem->parent() )
    {
//      printf("looking for 0x%x\n", nitem->parent() );
      nitem = (SPListViewItem *)nitem->parent();
    } 
  
    
    if( nitem )
    {
//      printf("here's the parent! 0x%x\n", nitem);
//      printf("  here's the rank of that parent: function name = %s\n",
//        nitem->text(1).ascii() );
//      matchSelectedItem( atoi( nitem->text(1).ascii() ) );
      matchSelectedItem( std::string(nitem->text(1).ascii()) );
    }
  }
}


void
pcStatsPanel::matchSelectedItem(std::string selected_function )
{
  bool foundFLAG = FALSE;
//  printf ("pcStatsPanel::matchSelectedItem() = %s\n", selected_function.c_str() );

std::vector<item_type>::const_iterator it = data_items.begin();
char timestr[50];
for( ; it != data_items.end(); it++ )
{
//  printf("%s %f\n", it->first.c_str(), it->second );
  if( selected_function == it->first.getName()  )
  {
//    printf("FOUND IT!\n");
    Optional<Statement> definition = it->first.getDefinition();
    if(definition.hasValue())
    {
      std::cout << " (" << definition.getValue().getPath()
              << ", " << definition.getValue().getLine() << ")";
      std::cout << std::endl;
      break;
    }
    fprintf(stderr, "No function definition for this entry.   Unable to position source.\n");
  }
}

//  SourceObject *spo = new SourceObject(it->first.getName().c_str(), it->first.getDefinition().getValue().getPath(), it->first.getDefinition().getValue().getLine(), TRUE, highlightList);
  SourceObject *spo = new SourceObject(it->first.getName().c_str(), it->first.getDefinition().getValue().getPath(), it->first.getDefinition().getValue().getLine(), TRUE, NULL);



  if( broadcast((char *)spo, NEAREST_T) == 0 )
  { // No source view up...
    char *panel_type = "Source Panel";
//Find the nearest toplevel and start placement from there...
    Panel *p = getPanelContainer()->dl_create_and_add_panel(panel_type, NULL, (void *)groupID);
    if( p != NULL ) 
    {
      p->listener((void *)spo);
    }
  }
}

template <class T>
struct sort_ascending : public std::binary_function<T,T,bool> {
    bool operator()(const T& x, const T& y) const {
        return x.second < y.second;
    }
};
template <class T>
struct sort_decending : public std::binary_function<T,T,bool> {
    bool operator()(const T& x, const T& y) const {
        return x.second > y.second;
    }
};


void
pcStatsPanel::updateStatsPanelBaseData(void *expr, int expID, QString experiment_name)
{
//  printf("pcStatsPanel::updateStatsPanelBaseData() entered.\n");

  StatsPanelBase::updateStatsPanelBaseData(expr, expID, experiment_name);

  
  SPListViewItem *lvi;
  columnList.clear();
//printf("This should be overloaded in pcStatsPanel....\n");

ExperimentObject *eo = Find_Experiment_Object((EXPID)expID);
if( eo && eo->FW() )
{
  Experiment *fw_experiment = eo->FW();
// Evaluate the collector's time metric for all functions in the thread
SmartPtr<std::map<Function, double> > data;
ThreadGroup tgrp = fw_experiment->getThreads();
ThreadGroup::iterator ti = tgrp.begin();
Thread t1 = *ti;
CollectorGroup cgrp = fw_experiment->getCollectors();
CollectorGroup::iterator ci = cgrp.begin();
Collector c1 = *ci;

Queries::GetMetricByFunctionInThread(c1, "time", t1, data);

// Display the results
  MetricHeaderInfoList metricHeaderInfoList;
  metricHeaderInfoList.push_back(new MetricHeaderInfo(QString("Time"), FLOAT_T));
  metricHeaderInfoList.push_back(new MetricHeaderInfo(QString("Function"), CHAR_T));
  if( metricHeaderTypeArray != NULL )
  {
    delete []metricHeaderTypeArray;
  }
  int header_count = metricHeaderInfoList.count();
  metricHeaderTypeArray = new int[header_count];

  int i=0;
  for( MetricHeaderInfoList::Iterator pit = metricHeaderInfoList.begin(); pit != metricHeaderInfoList.end(); ++pit )
  { 
    MetricHeaderInfo *mhi = (MetricHeaderInfo *)*pit;
    QString s = mhi->label;
    lv->addColumn( s );
    metricHeaderTypeArray[i] = mhi->type;
  
    columnList.push_back( s );
    i++;
  }


for(std::map<Function, double>::const_iterator
        item = data->begin(); item != data->end(); ++item)
{
  data_items.push_back( std::pair<Function, double>(item->first, item->second ) );
}

// std::sort(data_items.begin(), data_items.end(), sort_ascending<item_type>());
std::sort(data_items.begin(), data_items.end(), sort_decending<item_type>());

std::vector<item_type>::const_iterator it = data_items.begin();
char timestr[50];
for( ; it != data_items.end(); it++ )
{
//  printf("%s %f\n", it->first.c_str(), it->second );
  sprintf(timestr, "%f", it->second);
    lvi =  new SPListViewItem( this, lv, timestr,  it->first.getName().c_str() );
}

}
}

#ifdef PRINT_DEBUG
#include "SS_Input_Manager.hxx"
void
pcStatsPanel::PrintView(int expID)
{

printf("pcStatsPanel::PrintView(%d) entered\n", expID );
  ExperimentObject *eo = Find_Experiment_Object((EXPID)expID);
  if( eo && eo->FW() )
  {
    Experiment *fw_experiment = eo->FW();
    // Evaluate the collector's time metric for all functions in the thread
    SmartPtr<std::map<Function, double> > data;
    ThreadGroup tgrp = fw_experiment->getThreads();
    ThreadGroup::iterator ti = tgrp.begin();
    Thread t1 = *ti;
    CollectorGroup cgrp = fw_experiment->getCollectors();
    CollectorGroup::iterator ci = cgrp.begin();
    Collector c1 = *ci;
  
    Queries::GetMetricByFunctionInThread(c1, "time", t1, data);
  
    for(std::map<Function, double>::const_iterator
          item = data->begin(); item != data->end(); ++item)
    {
      printf("%20f %20s \n", item->second,  item->first.getName().c_str() );
    }
  }

printf("pcStatsPanel::PrintView(%d) finished\n", expID );
}


#endif // PRINT_DEBUG

/*******************************************************************************
** Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
**
** This library is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free
** Software Foundation; either version 2.1 of the License, or (at your option)
** any later version.
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
** details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/


#include "SS_Input_Manager.hxx"
#include "MPITCollector.hxx"
#include "MPITDetail.hxx"

// There are 2 reserved locations in the predefined-temporay table.
// Additional items may be defined for individual collectors.

// These are needed to manage mpit collector data.
#define start_temp 2
#define stop_temp 3
#define min_temp 4
#define max_temp 5
#define cnt_temp 6
#define ssq_temp 7

#define source_temp 8
#define destination_temp 9
#define size_temp 10
#define tag_temp 11
#define communicator_temp 12
#define datatype_temp 13
#define retval_temp 14

// mpit view

#define def_MPIT_values \
            Time start = Time::TheEnd(); \
            Time end = Time::TheBeginning(); \
            int64_t cnt = 0; \
            double sum = 0.0; \
            double vmax = 0.0; \
            double vmin = LONG_MAX; \
            double sum_squares = 0.0; \
            int64_t detail_source = 0; \
            int64_t detail_destination = 0; \
            uint64_t detail_size = 0; \
            int64_t detail_tag = 0; \
            int64_t detail_communicator = 0; \
            int64_t detail_datatype = 0; \
            int64_t detail_retval = 0;

#define get_MPIT_values(vi) \
              double v = (*vi).dm_time; \
              start = min(start,(*vi).dm_interval.getBegin()); \
              end = max(end,(*vi).dm_interval.getEnd()); \
              cnt ++; \
              vmin = min(vmin,v); \
              vmax = max(vmax,v); \
              sum += v; \
              sum_squares += v * v; \
              detail_source = (*vi).dm_source; \
              detail_destination = (*vi).dm_destination; \
              detail_size += (*vi).dm_size; \
              detail_tag = (*vi).dm_tag; \
              detail_communicator = (*vi).dm_communicator; \
              detail_datatype = (*vi).dm_datatype; \
              detail_retval = (*vi).dm_retval;

#define set_MPIT_values  \
              if (num_temps > VMulti_sort_temp) (*vcs)[VMulti_sort_temp] = NULL; \
              if (num_temps > start_temp) {  \
                double x = (start-base_time) / 1000000000.0; \
                (*vcs)[start_temp] = CRPTR (x); \
              } \
              if (num_temps > stop_temp) { \
                double x = (end-base_time) / 1000000000.0; \
                (*vcs)[stop_temp] = CRPTR (x); \
              } \
              if (num_temps > VMulti_time_temp) (*vcs)[VMulti_time_temp] = CRPTR (vmin); \
              if (num_temps > min_temp) (*vcs)[min_temp] = CRPTR (vmax); \
              if (num_temps > max_temp) (*vcs)[max_temp] = CRPTR (sum); \
              if (num_temps > cnt_temp) (*vcs)[cnt_temp] = CRPTR (cnt); \
              if (num_temps > ssq_temp) (*vcs)[ssq_temp] = CRPTR (sum_squares); \
              if (num_temps > source_temp) (*vcs)[source_temp] = CRPTR (detail_source); \
              if (num_temps > destination_temp) (*vcs)[destination_temp] = CRPTR (detail_destination); \
              if (num_temps > size_temp) (*vcs)[size_temp] = CRPTR (detail_size); \
              if (num_temps > tag_temp) (*vcs)[tag_temp] = CRPTR (detail_tag); \
              if (num_temps > communicator_temp) (*vcs)[communicator_temp] = CRPTR (detail_communicator); \
              if (num_temps > datatype_temp) (*vcs)[datatype_temp] = CRPTR (detail_datatype); \
              if (num_temps > retval_temp) (*vcs)[retval_temp] = CRPTR (detail_retval);


static bool MPIT_Trace_Report(
              CommandObject *cmd, ExperimentObject *exp, int64_t topn,
              ThreadGroup& tgrp, std::vector<Collector>& CV, std::vector<std::string>& MV,
              std::vector<ViewInstruction *>& IV, std::vector<std::string>& HV,
              std::list<CommandResult *>& view_output) {

  int64_t num_temps = max ((int64_t)VMulti_time_temp, Find_Max_Temp(IV)) + 1;
  bool TraceBack_Order = Determine_TraceBack_Ordering (cmd);
  Collector collector = CV[0];
  std::string metric = MV[0];
  std::vector<std::pair<CommandResult *,
                        SmartPtr<std::vector<CommandResult *> > > > c_items;

 // Get the list of desired functions.
  std::set<Function> objects;
  Determine_Objects ( cmd, exp, tgrp, objects);

  if (objects.empty()) {
    return false;
  }

 // Acquire base set of metric values.
  try {
    collector.lockDatabase();
    Extent databaseExtent = exp->FW()->getPerformanceDataExtent();
    Time base_time = databaseExtent .getTimeInterval().getBegin();

    SmartPtr<std::map<Function,
                      std::map<Framework::StackTrace,
                               std::vector<MPITDetail> > > > raw_items;
    GetMetricInThreadGroup (collector, metric, tgrp, objects, raw_items);
        std::map<Function, std::map<Framework::StackTrace, std::vector<MPITDetail> > >::iterator fi;
        for (fi = raw_items->begin(); fi != raw_items->end(); fi++) {
          std::map<Framework::StackTrace, std::vector<MPITDetail> >:: iterator si;
          for (si = (*fi).second.begin(); si != (*fi).second.end(); si++) {
            CommandResult *base_CSE = NULL;
            Framework::StackTrace st = (*si).first;
            std::vector<MPITDetail>::iterator vi;
            for (vi = (*si).second.begin(); vi != (*si).second.end(); vi++) {
              def_MPIT_values
              get_MPIT_values(vi)

            // Use macro to copy desired values.  
              SmartPtr<std::vector<CommandResult *> > vcs
                       = Framework::SmartPtr<std::vector<CommandResult *> >(
                                   new std::vector<CommandResult *>(num_temps)
                                   );
              set_MPIT_values 

             // Construct the type-independent return entry.
              CommandResult *CSE;
              if (base_CSE == NULL) {
                SmartPtr<std::vector<CommandResult *> > call_stack = Construct_CallBack (TraceBack_Order, false, st);
                base_CSE = new CommandResult_CallStackEntry (call_stack, TraceBack_Order);
                CSE = base_CSE;
              } else {
                CSE = Dup_CommandResult (base_CSE);
              }
              c_items.push_back(std::make_pair(CSE, vcs));
            }
          }
        }
  }
  catch (const Exception& error) {
    Mark_Cmd_With_Std_Error (cmd, error);
    collector.unlockDatabase();
    return false;
  }

  collector.unlockDatabase();

 // Generate the report.
  return Generic_Multi_View (cmd, exp, topn, tgrp, CV, MV, IV, HV, VFC_Trace, c_items, view_output);
}

static bool MPIT_Function_Report(
              CommandObject *cmd, ExperimentObject *exp, int64_t topn,
              ThreadGroup& tgrp, std::vector<Collector>& CV, std::vector<std::string>& MV,
              std::vector<ViewInstruction *>& IV, std::vector<std::string>& HV,
              std::list<CommandResult *>& view_output) {

  int64_t num_temps = max ((int64_t)VMulti_time_temp, Find_Max_Temp(IV)) + 1;
  Collector collector = CV[0];
  std::string metric = MV[0];
  std::vector<std::pair<CommandResult *,
                        SmartPtr<std::vector<CommandResult *> > > > c_items;

 // Get the list of desired functions.
  std::set<Function> objects;
  Determine_Objects ( cmd, exp, tgrp, objects);

  if (objects.empty()) {
    return false;
  }

  try {
    collector.lockDatabase();
    Extent databaseExtent = exp->FW()->getPerformanceDataExtent();
    Time base_time = databaseExtent .getTimeInterval().getBegin();

    SmartPtr<std::map<Function,
                      std::map<Framework::StackTrace,
                               std::vector<MPITDetail> > > > raw_items;
    GetMetricInThreadGroup (collector, metric, tgrp, objects, raw_items);
       // Combine all the items for each function.
        std::map<Function, std::map<Framework::StackTrace, std::vector<MPITDetail> > >::iterator fi;
        for (fi = raw_items->begin(); fi != raw_items->end(); fi++) {
          def_MPIT_values

          std::map<Framework::StackTrace, std::vector<MPITDetail> >:: iterator si;
          for (si = (*fi).second.begin(); si != (*fi).second.end(); si++) {
            std::vector<MPITDetail>::iterator vi;
            for (vi = (*si).second.begin(); vi != (*si).second.end(); vi++) {
              get_MPIT_values(vi)
            }
          }

         // Use macro to copy desired values.  
          SmartPtr<std::vector<CommandResult *> > vcs
                   = Framework::SmartPtr<std::vector<CommandResult *> >(
                               new std::vector<CommandResult *>(num_temps)
                               );
          set_MPIT_values 

         // Construct the type-independent return entry.
         // Construct callstack for last entry in the stack trace.
          Function F = (*fi).first;
          std::map<Framework::StackTrace,
                   std::vector<MPITDetail> >::iterator first_si = 
                                      (*fi).second.begin();
          Framework::StackTrace st = (*first_si).first;
          std::set<Statement> T = st.getStatementsAt(st.size()-1);

          SmartPtr<std::vector<CommandResult *> > call_stack =
                   Framework::SmartPtr<std::vector<CommandResult *> >(
                               new std::vector<CommandResult *>()
                               );
          call_stack->push_back(new CommandResult_Function (F, T));
          CommandResult *CSE = new CommandResult_CallStackEntry (call_stack);
          c_items.push_back(std::make_pair(CSE, vcs));
        }
  }
  catch (const Exception& error) {
    Mark_Cmd_With_Std_Error (cmd, error);
    collector.unlockDatabase();
    return false;
  }

  collector.unlockDatabase();

 // Generate the report.
  return Generic_Multi_View (cmd, exp, topn, tgrp, CV, MV, IV, HV, VFC_Function, c_items, view_output);
}

static bool MPIT_CallStack_Report (
              CommandObject *cmd, ExperimentObject *exp, int64_t topn,
              ThreadGroup& tgrp, std::vector<Collector>& CV, std::vector<std::string>& MV,
              std::vector<ViewInstruction *>& IV, std::vector<std::string>& HV,
              std::list<CommandResult *>& view_output) {

  int64_t num_temps = max ((int64_t)VMulti_time_temp, Find_Max_Temp(IV)) + 1;
  bool TraceBack_Order = Determine_TraceBack_Ordering (cmd);
  Collector collector = CV[0];
  std::string metric = MV[0];
  std::vector<std::pair<CommandResult *,
                        SmartPtr<std::vector<CommandResult *> > > > c_items;
  bool add_stmts = (!Look_For_KeyWord(cmd, "ButterFly") ||
                    Look_For_KeyWord(cmd, "FullStack") ||
                    Look_For_KeyWord(cmd, "FullStacks"));

 // Get the list of desired functions.
  std::set<Function> objects;
  Determine_Objects ( cmd, exp, tgrp, objects);

  if (objects.empty()) {
    return false;
  }

  try {
    collector.lockDatabase();
    Extent databaseExtent = exp->FW()->getPerformanceDataExtent();
    Time base_time = databaseExtent .getTimeInterval().getBegin();

    SmartPtr<std::map<Function,
                      std::map<Framework::StackTrace,
                               std::vector<MPITDetail> > > > raw_items;
    GetMetricInThreadGroup (collector, metric, tgrp, objects, raw_items);
       // Construct complete call stack
        std::map<Function,
                 std::map<Framework::StackTrace,
                          std::vector<MPITDetail> > >::iterator fi;
        for (fi = raw_items->begin(); fi != raw_items->end(); fi++) {
         // Foreach MPIT function ...
          std::map<Framework::StackTrace,
                   std::vector<MPITDetail> >::iterator sti;
          for (sti = (*fi).second.begin(); sti != (*fi).second.end(); sti++) {
           // Foreach call stack ...
            def_MPIT_values

            int64_t len = (*sti).second.size();
            for (int64_t i = 0; i < len; i++) {
             // Combine all the values.
              get_MPIT_values(&(*sti).second[i])
            }

            SmartPtr<std::vector<CommandResult *> > vcs
                     = Framework::SmartPtr<std::vector<CommandResult *> >(
                                 new std::vector<CommandResult *>(num_temps)
                                 );
           // Use macro to copy desired values.  
            set_MPIT_values 

           // Construct the type-independent return entry.
            Framework::StackTrace st = (*sti).first;
            SmartPtr<std::vector<CommandResult *> > call_stack = Construct_CallBack (TraceBack_Order, add_stmts, st);
            CommandResult *CSE = new CommandResult_CallStackEntry (call_stack, TraceBack_Order);
            c_items.push_back(std::make_pair(CSE, vcs));
          }
        }
  }
  catch (const Exception& error) {
    Mark_Cmd_With_Std_Error (cmd, error);
    collector.unlockDatabase();
    return false;
  }

  collector.unlockDatabase();

 // Generate the report.
  return Generic_Multi_View (cmd, exp, topn, tgrp, CV, MV, IV, HV, VFC_CallStack,c_items, view_output);
}

static std::string allowed_mpit_V_options[] = {
  "Function",
  "Functions",
  "Statement",
  "Statements",
  "Trace",
  "ButterFly",
  "CallTree",
  "CallTrees",
  "TraceBack",
  "TraceBacks",
  "FullStack",
  "FullStacks",
  "DontExpand",
  "Summary",
  ""
};

static void define_mpit_columns (
            CommandObject *cmd,
            std::vector<ViewInstruction *>& IV,
            std::vector<std::string>& HV,
            View_Form_Category vfc) {

 // Define combination instructions for predefined temporaries.
  IV.push_back(new ViewInstruction (VIEWINST_Add, VMulti_sort_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Min, start_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Max, stop_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Add, VMulti_time_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Min, min_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Max, max_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Add, cnt_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Add, ssq_temp));

 // Most detail fields are not combinable in a meaningful way.
  IV.push_back(new ViewInstruction (VIEWINST_Add, size_temp));

  OpenSpeedShop::cli::ParseResult *p_result = cmd->P_Result();
  vector<ParseRange> *p_slist = p_result->getexpMetricList();
  bool Generate_Summary = (Look_For_KeyWord(cmd, "Summary") & !Look_For_KeyWord(cmd, "ButterFly"));

  if (Generate_Summary) {
   // Add display of the summary time.
    IV.push_back(new ViewInstruction (VIEWINST_Display_Summary));
  }

  int64_t last_column = 0;
  if (p_slist->begin() != p_slist->end()) {
   // Add modifiers to output list.
    int64_t i = 0;
    bool time_metric_selected = false;
    vector<ParseRange>::iterator mi;
    for (mi = p_slist->begin(); mi != p_slist->end(); mi++) {
      bool column_is_DateTime = false;
      parse_range_t *m_range = (*mi).getRange();
      std::string C_Name;
      std::string M_Name;
      if (m_range->is_range) {
        C_Name = m_range->start_range.name;
        M_Name = m_range->end_range.name;
      } else {
        M_Name = m_range->start_range.name;
      }

     // Try to match the name with built in values.
      if (M_Name.length() > 0) {
        // Select temp values for columns and build column headers
        if (!time_metric_selected &&
            !strcasecmp(M_Name.c_str(), "exclusive_times") ||
            !strcasecmp(M_Name.c_str(), "exclusive_details")) {
         // display sum of times
          time_metric_selected = true;
          IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, VMulti_time_temp));
          HV.push_back("Exclusive Time");
          last_column++;
        } else if (!time_metric_selected &&
                   !strcasecmp(M_Name.c_str(), "inclusive_times") ||
                   !strcasecmp(M_Name.c_str(), "inclusive_details")) {
         // display times
          time_metric_selected = true;
          IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, VMulti_time_temp));
          HV.push_back("Inclusive Time");
          last_column++;
        } else if (!strcasecmp(M_Name.c_str(), "min")) {
         // display min time
          IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, min_temp));
          HV.push_back("Min Time");
          last_column++;
        } else if (!strcasecmp(M_Name.c_str(), "max")) {
         // display max time
          IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, max_temp));
          HV.push_back("Max Time");
          last_column++;
        } else if ( !strcasecmp(M_Name.c_str(), "count") ||
                    !strcasecmp(M_Name.c_str(), "counts") ||
                    !strcasecmp(M_Name.c_str(), "call") ||
                    !strcasecmp(M_Name.c_str(), "calls") ) {
         // display total counts
          IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, cnt_temp));
          HV.push_back("Number of Calls");
          last_column++;
        } else if (!strcasecmp(M_Name.c_str(), "average")) {
         // average time is calculated from two temps: sum and total counts.
          IV.push_back(new ViewInstruction (VIEWINST_Display_Average_Tmp, last_column, VMulti_time_temp, cnt_temp));
          HV.push_back("Average Time");
          last_column++;
        } else if (!strcasecmp(M_Name.c_str(), "percent")) {
         // percent is calculate from 2 temps: time for this row and total time.
          IV.push_back(new ViewInstruction (VIEWINST_Display_Percent_Tmp, last_column, VMulti_time_temp));
          HV.push_back("% of Total");
          last_column++;
        } else if (!strcasecmp(M_Name.c_str(), "stddev")) {
         // The standard deviation is calculated from 3 temps: sum, sum of squares and total counts.
          IV.push_back(new ViewInstruction (VIEWINST_Display_StdDeviation_Tmp, last_column,
                                            VMulti_time_temp, ssq_temp, cnt_temp));
          HV.push_back("Standard Deviation");
          last_column++;
        } else if (!strcasecmp(M_Name.c_str(), "start_time")) {
          if (vfc == VFC_Trace) {
           // display start time
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, start_temp));
            HV.push_back("Start Time");
            last_column++;
            column_is_DateTime = true;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m start_time' only supported for '-v Trace' option.");
          }
        } else if (!strcasecmp(M_Name.c_str(), "stop_time")) {
          if (vfc == VFC_Trace) {
           // display stop time
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, stop_temp));
            HV.push_back("Stop Time");
            last_column++;
            column_is_DateTime = true;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m stop_time' only supported for '-v Trace' option.");
          }
        } else if (!strcasecmp(M_Name.c_str(), "source")) {
          if (vfc == VFC_Trace) {
           // display source rank
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, source_temp));
            HV.push_back("Source Rank");
            last_column++;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m source' only supported for '-v Trace' option.");
          }
        } else if (!strcasecmp(M_Name.c_str(), "destination")) {
          if (vfc == VFC_Trace) {
           // display destination rank
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, destination_temp));
            HV.push_back("Destination Rank");
            last_column++;
            last_column++;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m destination' only supported for '-v Trace' option.");
          }
        } else if (!strcasecmp(M_Name.c_str(), "size")) {
         // display size of message
          IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, size_temp));
          HV.push_back("Message Size");
          last_column++;
        } else if (!strcasecmp(M_Name.c_str(), "tag")) {
          if (vfc == VFC_Trace) {
           // display tag of the message
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, tag_temp));
            HV.push_back("Message Tag");
            last_column++;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m tag' only supported for '-v Trace' option.");
          }
        } else if (!strcasecmp(M_Name.c_str(), "communicator")) {
          if (vfc == VFC_Trace) {
           // display communicator used
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, communicator_temp));
            HV.push_back("Communicator Used");
            last_column++;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m communicator' only supported for '-v Trace' option.");
          }
        } else if (!strcasecmp(M_Name.c_str(), "datatype")) {
          if (vfc == VFC_Trace) {
           // display data type of the message
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, datatype_temp));
            HV.push_back("Message Type");
            last_column++;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m datatype' only supported for '-v Trace' option.");
          }
        } else if (!strcasecmp(M_Name.c_str(), "retval")) {
          if (vfc == VFC_Trace) {
           // display enumerated return value
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, retval_temp));
            HV.push_back("Return Value");
            last_column++;
          } else {
            Mark_Cmd_With_Soft_Error(cmd,"Warning: '-m retval' only supported for '-v Trace' option.");
          }
        } else {
          Mark_Cmd_With_Soft_Error(cmd,"Warning: Unsupported option, '-m " + M_Name + "'");
        }
      }
      if (last_column == 1) {
        IV.push_back(new ViewInstruction (VIEWINST_Sort_Ascending, (int64_t)(column_is_DateTime) ? 1 : 0));
      }
    }
  } else {
   // If nothing is requested ...
    if (vfc == VFC_Trace) {
      // Insert start times, end times, source rank and destination rank into report.
      IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, start_temp));
      HV.push_back("Start Time");
      IV.push_back(new ViewInstruction (VIEWINST_Sort_Ascending, 1)); // final report in ascending time order
      last_column++;
      IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, stop_temp));
      HV.push_back("Stop Time");
      last_column++;
    }
   // Always display elapsed time.
    IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column, VMulti_time_temp));
    HV.push_back("Exclusive Time");
  }
}

static bool mpit_definition (CommandObject *cmd, ExperimentObject *exp, int64_t topn,
                             ThreadGroup& tgrp, std::vector<Collector>& CV, std::vector<std::string>& MV,
                             std::vector<ViewInstruction *>& IV, std::vector<std::string>& HV,
                             View_Form_Category vfc) {
    Assert (CV.begin() != CV.end());
    CV.erase(++CV.begin(), CV.end());  // Save the collector name
   // Clean the other vectors
    MV.erase(MV.begin(), MV.end());
    IV.erase(IV.begin(), IV.end());
    HV.erase(HV.begin(), HV.end());

    CollectorGroup cgrp = exp->FW()->getCollectors();
    Collector C = *CV.begin();
    if (cgrp.find(C) == std::set<Collector>::iterator(cgrp.end())) {
      std::string C_Name = C.getMetadata().getUniqueId();
      std::string s("The required collector, " + C_Name + ", was not used in the experiment.");
      Mark_Cmd_With_Soft_Error(cmd,s);
      return false;
    }
/* TEST
    std::string M_Name("exclusive_times");
TEST */
    std::string M_Name("exclusive_details");
    MV.push_back(M_Name);
    if (!Collector_Generates_Metric (*CV.begin(), M_Name)) {
      std::string s("The metrics required to generate the view are not available in the experiment.");
      Mark_Cmd_With_Soft_Error(cmd,s);
      return false;
    }

    Validate_V_Options (cmd, allowed_mpit_V_options);
    define_mpit_columns (cmd, IV, HV, vfc);

    return true;
}

static std::string VIEW_mpit_brief = "Mpit Report";
static std::string VIEW_mpit_short = "Report information about the calls to mpi functions.";
static std::string VIEW_mpit_long  = "\n\nA positive integer can be added to the end of the keyword"
                                      " 'mpit' to indicate the maximum number of items in the report."
                                      " When the '-v Trace' option is selected, the selected items are"
                                      " the ones that use the most time.  In all other cases"
                                      " the selection will be based on the values displayed in"
                                      " left most column of the report."
                                      "\n\nThe form of the information displayed can be controlled through"
                                      " the  '-v' option.  Except for the '-v Trace' option, the report will"
                                      " be sorted in descending order of the value in the left most column"
                                      " displayed on a line. [See '-m' option for controlling this field.]"
                                      "\n\t'-v Functions' will produce a summary report for each function."
                                      " This is the default report, if nothing else is requested."
                                      "\n\t'-v Trace' will produce a report of each call to an mpi function."
                                      " It will be sorted in ascending order of the starting time for the event."
                                      "\n\t'-v CallTrees' will produce report that expands the call stack for"
                                      " a trace, providing the sequence of functions called from the start of"
                                      " the program to the measured function."
                                      "\n\t'-v TraceBacks' will produce a report that expands the call stack for"
                                      " a trace, providing the sequence of functions called from the function"
                                      " back to the start of the program."
                                      "\n\tThe addition of 'FullStack' with either 'CallTrees' of 'TraceBacks'"
                                      " will cause the report to include the full call stack for each measured"
                                      " function.  Redundant portions of a call stack are suppressed if this"
                                      " option is not specified."
                                      "\n\tThe addition of 'Summary' to the '-v' option list will result in an"
                                      " additional line of output at"
                                      " the end of the report that summarizes the information in each column."
                                      "\n\t'-v ButterFly' along with a '-f <function_list>' will produce a report"
                                      " that summarizes the calls to a function and the calls from the function."
                                      " The calling functions will be listed before the named function and the"
                                      " called functions afterwards, by default, although the addition of"
                                      " 'TraceBacks' to the '-v' specifier will reverse this ordering."
                                      "\n\nThe information included in the report can be controlled with the"
                                      " '-m' option.  More than one item can be selected but only the items"
                                      " listed after the option will be printed and they will be printed in"
                                      " the order that they are listed."
                                      " If no '-m' option is specified, the default is equivalent to"
                                      " '-m start_time, end_time, exclusive times'."
                                      " Clearly, not every value will be meaningful with every '-v' option."
                                      " \n\t'-m exclusive_times' reports the wall clock time used in the event."
                                      " \n\t'-m min' reports the minimum time spent in the event."
                                      " \n\t'-m max' reports the maximum time spent in the event."
                                      " \n\t'-m average' reports the average time spent in the event."
                                      " \n\t'-m count' reports the number of times the event occured."
                                      " \n\t'-m percent' reports the percent of mpi time the event represents."
                                      " \n\t'-m stddev' reports the standard deviation of the average mpi time"
                                      " that the event represents."
                                      " \n\t'-m start_time' reports the starting time of the event."
                                      " \n\t'-m stop_time' reports the ending time of the event."
                                      " \n\t'-m source' reports the source rank of the event."
                                      " \n\t'-m destination' reports the destination rank of the event."
                                      " \n\t'-m size' reports the number of bytes in the message."
                                      " \n\t'-m tag' reports the tag of the event."
                                      " \n\t'-m communicator' reports the communicator used for the event."
                                      " \n\t'-m datatype' reports the data type of the message."
                                      " \n\t'-m retval' reports the return value of the event."
;
static std::string VIEW_mpit_example = "\texpView mpit\n"
                                       "\texpView -v Trace mpit10\n" 
                                       "\texpView -v Trace mpit100 -m start_time, inclusive_time, size\n";
static std::string VIEW_mpit_metrics[] =
  { "exclusive_details",
    "exclusive_times",
    "inclusive_details",
    "inclusive_times",
    ""
  };
static std::string VIEW_mpit_collectors[] =
  { "mpit",
    ""
  };
class mpit_view : public ViewType {

 public: 
  mpit_view() : ViewType ("mpit",
                          VIEW_mpit_brief,
                          VIEW_mpit_short,
                          VIEW_mpit_long,
                          VIEW_mpit_example,
                         &VIEW_mpit_metrics[0],
                         &VIEW_mpit_collectors[0],
                          true) {
  }
  virtual bool GenerateView (CommandObject *cmd, ExperimentObject *exp, int64_t topn,
                             ThreadGroup& tgrp, std::list<CommandResult *>& view_output) {
    std::vector<Collector> CV;
    std::vector<std::string> MV;
    std::vector<ViewInstruction *>IV;
    std::vector<std::string> HV;

    CV.push_back (Get_Collector (exp->FW(), "mpit"));  // Define the collector
    View_Form_Category vfc = Determine_Form_Category(cmd);
    if (mpit_definition (cmd, exp, topn, tgrp, CV, MV, IV, HV, vfc)) {

      if ((CV.size() == 0) ||
          (MV.size() == 0)) {
        Mark_Cmd_With_Soft_Error(cmd, "(There are no metrics specified to report.)");
        return false;   // There is no collector, return.
      }

      switch (Determine_Form_Category(cmd)) {
       case VFC_Trace:
        return MPIT_Trace_Report (cmd, exp, topn, tgrp, CV, MV, IV, HV, view_output);
       case VFC_CallStack:
        return MPIT_CallStack_Report (cmd, exp, topn, tgrp, CV, MV, IV, HV, view_output);
       case VFC_Function:
        return MPIT_Function_Report (cmd, exp, topn, tgrp, CV, MV, IV, HV, view_output);
      }
    }
    return false;
  }
};


// This is the only external entrypoint.
// Calls to the VIEWs needs to be done through the ViewType class objects.
extern "C" void mpit_view_LTX_ViewFactory () {
  Define_New_View (new mpit_view());
}

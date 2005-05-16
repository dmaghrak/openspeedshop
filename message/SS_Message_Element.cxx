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

/** @file
 *
 * Message related objects and routines.
 *
 */

#include <vector>
#include <string>
#include <iostream>

using namespace std;

#include "SS_Message_Element.hxx"

using namespace OpenSpeedShop;

/**
 * Constructor: SS_Message_Element::SS_Message_Element()
 * 
 *     
 *
 * @todo    Error handling.
 *
 */
SS_Message_Element::
SS_Message_Element() :
    dm_id_num	    	    (-1)
{

}
 
/**
 * Destructor: SS_Message_Element::~SS_Message_Element()
 * 
 *     
 *
 * @todo    Error handling.
 *
 */
SS_Message_Element::
~SS_Message_Element()
{
}
 
/**
 * Method: SS_Message_Element::set_keyword(char *keyword_str)
 * 
 * This is the string that this message is referenced
 * by when you type "help <keyword>".
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_keyword(char *keyword_str) 
{
    dm_keyword = keyword_str;
}

/**
 * Method: SS_Message_Element::set_keyword(string keyword_str)
 * 
 * This is the string that this message is referenced
 * by when you type "help <keyword>".
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_keyword(string keyword_str) 
{
    dm_keyword = keyword_str;
}

/**
 * Method: SS_Message_Element::set_keyword(string keyword_str,string topic_str)
 * 
 * This is the string that this message is referenced
 * by when you type "help <keyword>".
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_keyword(string keyword_str,string topic_str) 
{
    dm_keyword = keyword_str;
    dm_topic = topic_str;

}

/**
 * Method: SS_Message_Element::set_keyword(string keyword_str,string topic_str,int id)
 * 
 * This is the string that this message is referenced
 * by when you type "help <keyword>".
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_keyword(string keyword_str,string topic_str,int id) 
{
    dm_keyword = keyword_str;
    dm_topic = topic_str;
    dm_id_num = id;
}

/**
 * Method: SS_Message_Element::get_keyword()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
const string *
SS_Message_Element::
get_keyword() 
{
   return &dm_keyword;
}

/**
 * Method: SS_Message_Element::set_topic(char * topic_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_topic(char *topic_str) 
{
   dm_topic = topic_str;
}

/**
 * Method: SS_Message_Element::set_topic(string topic_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_topic(string topic_str) 
{
   dm_topic = topic_str;
}

/**
 * Method: SS_Message_Element::get_topic()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
const string *
SS_Message_Element::
get_topic() 
{
   return &dm_topic;
}

/**
 * Method: SS_Message_Element::add_related(char *related_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_related(char *related_str) 
{
   dm_related_keyword_list.push_back(related_str);
}

/**
 * Method: SS_Message_Element::add_related(string related_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_related(string related_str) 
{
   dm_related_keyword_list.push_back(related_str);
}

/**
 * Method: SS_Message_Element::get_related_list()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
vector<string> *
SS_Message_Element::
get_related_list() 
{
    return &dm_related_keyword_list;
}

/**
 * Method: SS_Message_Element::set_id(int id)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_id(int id) 
{
    dm_id_num = id;
}

/**
 * Method: SS_Message_Element::get_id()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
int
SS_Message_Element::
get_id() 
{
    return dm_id_num;
}

/**
 * Method: SS_Message_Element::get_brief()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
const string *
SS_Message_Element::
get_brief() 
{
   return &dm_brief_message;
}

/**
 * Method: SS_Message_Element::set_brief(char *brief_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_brief(char *brief_str) 
{
   dm_brief_message = brief_str;
}

/**
 * Method: SS_Message_Element::set_brief(string brief_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_brief(string brief_str) 
{
   dm_brief_message = brief_str;
}

/**
 * Method: SS_Message_Element::add_normal(char *normal_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_normal(char *normal_str) 
{
   dm_normal_message_list.push_back(normal_str);
}

/**
 * Method: SS_Message_Element::add_normal(string normal_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_normal(string normal_str) 
{
   dm_normal_message_list.push_back(normal_str);
}

/**
 * Method: SS_Message_Element::get_normal_list()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
vector<string> *
SS_Message_Element::
get_normal_list() 
{
    return &dm_normal_message_list;
}

/**
 * Method: SS_Message_Element::add_verbose(char *verbose_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_verbose(char *verbose_str) 
{
   dm_verbose_message_list.push_back(verbose_str);
}

/**
 * Method: SS_Message_Element::add_verbose(string verbose_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_verbose(string verbose_str) 
{
   dm_verbose_message_list.push_back(verbose_str);
}

/**
 * Method: SS_Message_Element::get_verbose_list()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
vector<string> *
SS_Message_Element::
get_verbose_list() 
{
    return &dm_verbose_message_list;
}

/**
 * Method: SS_Message_Element::add_example(char *example_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_example(char *example_str) 
{
   dm_example_list.push_back(example_str);
}

/**
 * Method: SS_Message_Element::add_example(string example_str)
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
add_example(string example_str) 
{
    dm_example_list.push_back(example_str);
}

/**
 * Method: SS_Message_Element::get_example_list()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
vector<string> *
SS_Message_Element::
get_example_list() 
{
    return &dm_example_list;
}

/**
 * Method: SS_Message_Element::set_element()
 * 
 * Use message_element_t to initialize element.
 * I see this as the usual method to use.
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
set_element(message_element_t *p_element) 
{

    // Numeric id
    if (p_element->errnum)
    	dm_id_num = p_element->errnum;
    
    // Look up name
    if (p_element->keyword)
    	set_keyword(p_element->keyword);
    
    // Related keywords
    for (int i=0;i<p_element->related_cnt;++i) {
    	add_related(p_element->related[i]);
    }
    
    // General topic
    if (p_element->topic)
    	set_topic(p_element->topic);

    // Brief, one line description
    if (p_element->brief)
    	set_brief(p_element->brief);

    // More than one line description
    for (int i=0;i<p_element->normal_cnt;++i) {
    	add_normal(p_element->normal[i]);
    }

    // A wordy explaination
    for (int i=0;i<p_element->detail_cnt;++i) {
    	add_verbose(p_element->detailed[i]);
    }

    // An example of usage
    for (int i=0;i<p_element->example_cnt;++i) {
    	add_example(p_element->example[i]);
    }

}

#if 0
/**
 * Method: SS_Message_Element::xx()
 * 
 * 
 * 
 *     
 * @return  void.
 *
 * @todo    Error handling.
 *
 */
void
SS_Message_Element::
xx() 
{

}

#endif


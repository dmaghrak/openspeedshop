#include <stdio.h>

#include <vector>
#include <iostream>

using namespace std;

#include "SS_Parse_Range.hxx"
#include "SS_Parse_Target.hxx"
#include "SS_Parse_Result.hxx"

using namespace OpenSpeedShop::cli;
/*
this is a negative test for ParseResult.GetExpId().

*/

int main()
{
	ParseResult p_parse_result;
	p_parse_result.SetExpId(5);
	if (p_parse_result.GetExpId() == 6)
		cout << "FAIL" << endl;
	else
		cout << "PASS" << endl;
	return 0;
}

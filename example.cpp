#include <iostream>
#include <string>
#include "xml.h"

using namespace std;

int main(int argc, char* argv[])
{
    common::xml::XmlDocument doc;
    try
    {
        if (doc.LoadXml("<?xml version=\"1.0\"?><xml test=\"ja\"><test><close><bla /></close></test><test>jfuhsiudfg sfdg</test><close><bla /></close><!--- Commentaar --><wouter><![CDATA[<h1>Titel</h1>]]></wouter><close><bla /></close></xml>"))
        {
            cout << doc.DocumentElement()->OuterXml() << endl;
            common::xml::XmlNode* match = doc.DocumentElement()->SelectSingleNode("//wouter");
            if (match != 0)
                cout << match->InnerText() << endl;
            else
                cout << "No name" << endl;
        }
        else
            cout << "Error loading xml" << endl;
    }
    catch (const string& err)
    {
        cout << err << endl;
    }
    
    return 0;
}

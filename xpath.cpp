#include "xml.h"
#include <iostream>

using namespace std;
using namespace common::xml;

XmlNodeList Matches(XmlNode* item, const char* xpath)
{
    XmlNodeList matches;

    string name;
    while (xpath[0] != '[' && xpath[0] != '/' && xpath[0] != '\0')
    {
        name += xpath[0];
        ++xpath;
    }

    // If there is a filter [] then we grab that into one string
    if (xpath[0] == '[')
    {
        ++xpath;    // skip [
        int level = 0;
        string filter;
        while (xpath[0] != ']' && level > 0)
        {
            if (xpath[0] == '[') level++;
            if (xpath[0] == ']') level--;
            filter + xpath[0];
            ++xpath;
        }
        ++xpath;    // skip ]
        // TODO: act on filter, for now we skip this.
    }
    else if (xpath[0] == '/')
    {
        ++xpath;
    }

    if (item->LocalName() == name)
    {
        // Are we at the end of the expression?
        if (xpath[0] != '\0')
        {
            for (XmlNodeList::iterator i = item->ChildNodes().begin(); i != item->ChildNodes().end(); ++i)
            {
                XmlNodeList childResults = Matches(*i, xpath);
                matches.insert(matches.end(), childResults.begin(), childResults.end());
            }
        }
        else
            matches.push_back(item);
    }

    return matches;
}

XmlNode* Match(XmlNode* item, const char* xpath)
{
    string name;
    while (xpath[0] != '[' && xpath[0] != '/' && xpath[0] != '\0')
    {
        name += xpath[0];
        ++xpath;
    }

    // If there is a filter [] then we grab that into one string
    if (xpath[0] == '[')
    {
        ++xpath;    // skip [
        int level = 0;
        string filter;
        while (xpath[0] != ']' && level > 0)
        {
            if (xpath[0] == '[') level++;
            if (xpath[0] == ']') level--;
            filter + xpath[0];
            ++xpath;
        }
        ++xpath;    // skip ]
        // TODO: act on filter, for now we skip this.
    }
    else if (xpath[0] == '/')
    {
        ++xpath;
    }

    if (item->LocalName() == name)
    {
        // Are we at the end of the expression?
        if (xpath[0] != '\0')
        {
            for (XmlNodeList::iterator i = item->ChildNodes().begin(); i != item->ChildNodes().end(); ++i)
            {
                XmlNode* childResults = Match(*i, xpath);
                if (childResults != 0)
                    return childResults;
            }
        }
        else
            return item;
    }

    return 0;
}

XmlNodeList XmlNode::SelectNodes(const string& sxpath)
{
    XmlNodeList matches;

    const char* xpath = sxpath.c_str();

    if (xpath[0] == '/' && xpath[1] == '/')
    {
        for (set<XmlNode*>::iterator i = this->_ownerDocument->_nodes.begin(); i != this->_ownerDocument->_nodes.end(); ++i)
        {
            XmlNodeList childResults = Matches(*i, xpath+2);
            matches.insert(matches.end(), childResults.begin(), childResults.end());
        }
    }
    else if (xpath[0] == '/')
    {
        XmlNodeList childResults = Matches(this->_ownerDocument->_documentElement, xpath+1);
        matches.insert(matches.end(), childResults.begin(), childResults.end());
    }
    else
    {
        XmlNodeList childResults = Matches(this, xpath);
        matches.insert(matches.end(), childResults.begin(), childResults.end());
    }

    return matches;
}

XmlNode* XmlNode::SelectSingleNode(const string& sxpath)
{
    const char* xpath = sxpath.c_str();

    if (xpath[0] == '/' && xpath[1] == '/')
    {
        for (set<XmlNode*>::iterator i = this->_ownerDocument->_nodes.begin(); i != this->_ownerDocument->_nodes.end(); ++i)
        {
            XmlNode* childResult = Match(*i, xpath+2);
            if (childResult != 0)
                return childResult;
        }
    }
    else if (xpath[0] == '/')
        return Match(this->_ownerDocument->_documentElement, xpath+1);
    else
        return Match(this, xpath);

    return 0;
}

XmlNodeList XmlDocument::SelectNodes(const std::string& xpath)
{
    if (this->_documentElement != 0)
        return this->_documentElement->SelectNodes(xpath);
    return XmlNodeList();
}

XmlNode* XmlDocument::SelectSingleNode(const std::string& xpath)
{
    if (this->_documentElement != 0)
        return this->_documentElement->SelectSingleNode(xpath);
    return 0;
}

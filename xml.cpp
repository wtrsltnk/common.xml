#include "xml.h"
#include <fstream>

using namespace std;
using namespace common::xml;

////////////////////////////////////////////////////////////////////////////////////
// XmlNode
////////////////////////////////////////////////////////////////////////////////////
XmlNode::XmlNode(XmlDocument* ownerDocument, XmlNode* parentNode, const string& localname)
    : _ownerDocument(ownerDocument), _parentNode(parentNode), _localName(localname)
{
    if (this->_ownerDocument != 0)
        this->_ownerDocument->_nodes.insert(this);
}

XmlNode::~XmlNode()
{
    this->ClearAttributes();
    this->ClearChildNodes();
    if (this->_ownerDocument != 0)
        this->_ownerDocument->_nodes.erase(this);
}

string XmlNode::InnerText()
{
    string result;

    for (XmlNodeList::iterator i = this->_childNodes.begin(); i != this->_childNodes.end(); i++)
        result += (*i)->InnerText();

    return result;
}

void XmlNode::InnerText(const string& innertext)
{
    this->ClearChildNodes();
    this->_childNodes.push_back(new XmlText(this->OwnerDocument(), this, innertext));
}

string XmlNode::InnerXml()
{
    string result;

    for (XmlNodeList::iterator i = this->_childNodes.begin(); i != this->_childNodes.end(); i++)
        result += (*i)->InnerXml();

    return result;
}

void XmlNode::InnerXml(const string& innerxml)
{
    this->ClearChildNodes();
    this->_childNodes = XmlNode::LoadXml(this->_ownerDocument, innerxml);
}

string XmlNode::OuterXml()
{
    string result = "<" + this->_localName;

    for (XmlAttributeCollection::iterator i = this->_attributes.begin(); i != this->_attributes.end(); ++i)
        result += " " + (*i).first + "=\"" + (*i).second->Value() + "\"";
    if (this->_childNodes.size() > 0)
    {
        result += ">";

        for (XmlNodeList::iterator i = this->_childNodes.begin(); i != this->_childNodes.end(); ++i)
            result += (*i)->OuterXml();

        result += "</" + this->_localName + ">";
    }
    else
        result += " />";

    return result;
}

void XmlNode::ClearAttributes()
{
    map<string, XmlAttribute*>::iterator it = this->_attributes.begin();
    while (it != this->_attributes.end())
    {
        XmlAttribute* attr = it->second;
        this->_attributes.erase(it);
        delete attr;
        it = this->_attributes.begin();
    }
}

void XmlNode::ClearChildNodes()
{
    while (this->_childNodes.empty() == false)
    {
        XmlNode* node = this->_childNodes.back();
        this->_childNodes.pop_back();
        delete node;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// XmlDeclaration
////////////////////////////////////////////////////////////////////////////////////
XmlDeclaration::XmlDeclaration(XmlDocument *ownerDocument, const vector<string> &tokens)
    : XmlNode(ownerDocument, 0, "<?xml Declaration ?>")
{
    this->_attributes = XmlNode::_LoadAttributes(ownerDocument, this, tokens);
}

XmlDeclaration::~XmlDeclaration()
{ }

string XmlDeclaration::OuterXml()
{
    string result = "<?xml";

    for (XmlAttributeCollection::iterator i = this->_attributes.begin(); i != this->_attributes.end(); i++)
        result += " " + (*i).first + "=\"" + (*i).second->Value() + "\"";

    result += "?>";

    return result;
}

////////////////////////////////////////////////////////////////////////////////////
// XmlCharacterData
////////////////////////////////////////////////////////////////////////////////////
XmlCharacterData::XmlCharacterData(XmlDocument* ownerDocument, XmlNode* parentNode, const string& data)
    : XmlNode(ownerDocument, parentNode, "<![CDATA[ CharacterData ]]>"), _data(data)
{ }

XmlCharacterData::~XmlCharacterData()
{ }

void XmlCharacterData::InnerText(const string& text)
{
    this->_data = text;
}

string XmlCharacterData::OuterXml()
{
    return "<![CDATA[" + this->_data + "]]>";
}

////////////////////////////////////////////////////////////////////////////////////
// XmlText
////////////////////////////////////////////////////////////////////////////////////
XmlText::XmlText(XmlDocument* ownerDocument, XmlNode* parentNode, const string& text)
    : XmlCharacterData(ownerDocument, parentNode, text)
{ }

XmlText::~XmlText()
{ }

string XmlText::OuterXml()
{
    return this->_data;
}

////////////////////////////////////////////////////////////////////////////////////
// XmlComment
////////////////////////////////////////////////////////////////////////////////////
XmlComment::XmlComment(XmlDocument* ownerDocument, XmlNode* parentNode, const string& comment)
    : XmlNode(ownerDocument, parentNode, "<!-- Comment -->"), _comment(comment)
{ }

XmlComment::~XmlComment()
{ }

void XmlComment::Comment(const string& comment)
{
    this->_comment = comment;
}

string XmlComment::OuterXml()
{
    return "<!--" + this->_comment + "-->";
}

////////////////////////////////////////////////////////////////////////////////////
// XmlAttribute
////////////////////////////////////////////////////////////////////////////////////
XmlAttribute::XmlAttribute(XmlDocument* ownerDocument, XmlNode* parentNode, const string& key, const string& value)
    : XmlNode(ownerDocument, parentNode, string("@") + key), _key(key), _value(value)
{ }

XmlAttribute::~XmlAttribute()
{ }

void XmlAttribute::Value(const string& value)
{
    this->_value = value;
}

////////////////////////////////////////////////////////////////////////////////////
// XmlDocument
////////////////////////////////////////////////////////////////////////////////////
XmlDocument::XmlDocument()
    : _documentElement(0), _declaration(0)
{ }

XmlDocument::~XmlDocument()
{
    if (this->_declaration != 0)
        delete this->_declaration;
    this->_declaration = 0;
    if (this->_documentElement != 0)
        delete this->_documentElement;
    this->_documentElement = 0;
}

bool XmlDocument::Load(const string& filename)
{
    ifstream ifs(filename.c_str());

    string content((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

    return this->LoadXml(content);
}

bool XmlDocument::LoadXml(const string& xml)
{
    XmlNodeList result = XmlNode::LoadXml(this, xml);

    if (result.size() > 2)
        return false;

    if (result.size() > 1 && result[0]->LocalName() != "<?xml Declaration ?>")
        return false;

    if (result.size() == 2)
        this->_declaration = result[0];

    this->_documentElement = result[result.size()-1];

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// priv::XmlParser
////////////////////////////////////////////////////////////////////////////////////
priv::XmlParser::XmlParser(const std::string& xml)
    : _data(xml.c_str()), _cursor(_data), _size(xml.size())
{ }

priv::XmlParser::XmlParser(const priv::XmlParser& other)
    : _data(other._data), _cursor(other._cursor), _size(other._size)
{ }

priv::XmlParser::~XmlParser()
{ }

priv::XmlParser& priv::XmlParser::operator = (const XmlParser& other)
{
    this->_data = other._data;
    this->_cursor = other._cursor;
    this->_size = other._size;

    return (*this);
}

priv::XmlParser& priv::XmlParser::operator += (int count)
{
    if (((this->_cursor + count) - this->_data) < this->_size)
        this->_cursor += count;

    return (*this);
}

char priv::XmlParser::operator ++ ()
{
    if (this->HasToken())
        this->_cursor++;

    return this->Character();
}

bool priv::XmlParser::operator == (char c) const
{
    return (this->_cursor[0] == c);
}

bool priv::XmlParser::HasToken()
{
    return ((this->_cursor - this->_data) < this->_size);
}

void priv::XmlParser::SkipSpaces()
{
    while (this->HasToken() && this->_cursor[0] <= ' ')
        this->_cursor++;
}

string priv::XmlParser::CurrentToken()
{
    string result;

    int at = (this->_cursor - this->_data);
    int charsLeft = this->_size - at;

    if (charsLeft >= 2
            && this->_cursor[0] == '<'
            && this->_cursor[1] == '/')
    {
        result += this->_cursor[0];
        result += this->_cursor[1];
    }
    else if (charsLeft >= 5
             && this->_cursor[0] == '<'
             && this->_cursor[1] == '?'
             && this->_cursor[2] == 'x'
             && this->_cursor[3] == 'm'
             && this->_cursor[4] == 'l')
    {
        result += this->_cursor[0];
        result += this->_cursor[1];
        result += this->_cursor[2];
        result += this->_cursor[3];
        result += this->_cursor[4];
    }
    else if (charsLeft >= 9
             && this->_cursor[0] == '<'
             && this->_cursor[1] == '!'
             && this->_cursor[2] == '['
             && this->_cursor[3] == 'C'
             && this->_cursor[4] == 'D'
             && this->_cursor[5] == 'A'
             && this->_cursor[6] == 'T'
             && this->_cursor[7] == 'A'
             && this->_cursor[8] == '[')
    {
        result += this->_cursor[0];
        result += this->_cursor[1];
        result += this->_cursor[2];
        result += this->_cursor[3];
        result += this->_cursor[4];
        result += this->_cursor[5];
        result += this->_cursor[6];
        result += this->_cursor[7];
        result += this->_cursor[8];
    }
    else if (charsLeft >= 4
             && this->_cursor[0] == '<'
             && this->_cursor[1] == '!'
             && this->_cursor[2] == '-'
             && this->_cursor[3] == '-')
    {
        result += this->_cursor[0];
        result += this->_cursor[1];
        result += this->_cursor[2];
        result += this->_cursor[3];
    }
    else if (charsLeft >= 3
             && ((this->_cursor[0] == ']' && this->_cursor[0] == ']') || (this->_cursor[1] == '-' && this->_cursor[1] == '-')) && this->_cursor[2] == '>')
    {
        result += this->_cursor[0];
        result += this->_cursor[1];
        result += this->_cursor[2];
    }
    else if (this->_cursor[0] == '<' || this->_cursor[0] == '=' || this->_cursor[0] == '>')
        result += this->_cursor[0];
    else if (charsLeft >= 2
             && (this->_cursor[0] == '/' || this->_cursor[0] == '?')
             && this->_cursor[1] == '>')
    {
        result += this->_cursor[0];
        result += this->_cursor[1];
    }
    else if (this->_cursor[0] == '\"')
    {
        result += this->_cursor[0];
        int i = 1;
        while (this->_cursor[i] != '\"')
            result += this->_cursor[i++];
        result += this->_cursor[i];
    }
    else
    {
        int i = 0;
        while (this->_cursor[i] > ' ' && this->_cursor[i] != '<' && this->_cursor[i] != '=' && this->_cursor[i] != '/' && this->_cursor[i] != '>')
            result += this->_cursor[i++];
    }

    return result;
}

string priv::XmlParser::NextToken()
{
    string token = this->CurrentToken();
    (*this) += token.size();
    this->SkipSpaces();
    return this->CurrentToken();
}

char priv::XmlParser::Character()
{
    return this->_cursor[0];
}

////////////////////////////////////////////////////////////////////////////////////
// Xml Loading code
////////////////////////////////////////////////////////////////////////////////////
XmlNodeList XmlNode::LoadXml(XmlDocument* ownerDocument, const string& xml)
{
    XmlNodeList result;
    priv::XmlParser parser(xml);

    parser.SkipSpaces();

    while (parser.HasToken() && parser.Character() == '<')
    {
        XmlNode* node = XmlNode::_LoadNode(ownerDocument, 0, parser);
        if (node != 0)
            result.push_back(node);
        parser.SkipSpaces();
    }

    return result;
}

XmlNode* XmlNode::_LoadNode(XmlDocument* ownerDocument, XmlNode* parentNode, priv::XmlParser& parser)
{
    parser.SkipSpaces();

    // <?xml ?>
    if (parser.CurrentToken() == "<?xml")
    {
        vector<string> tokens;
        string token = parser.NextToken();    // skip <?xml
        while (token != "?>")
        {
            tokens.push_back(token);
            token = parser.NextToken();
        }
        parser.NextToken();

        return new XmlDeclaration(ownerDocument, tokens);
    }
    // <![CDATA[ ]]>
    else if (parser.CurrentToken() == "<![CDATA[")
    {
        parser.NextToken();    // skip <![CDATA[

        string data;
        while (parser.CurrentToken() != "]]>")
        {
            data += parser.Character();
            ++parser;
        }
        parser.NextToken();

        return new XmlCharacterData(ownerDocument, parentNode, data);
    }
    // <!-- -->
    else if (parser.CurrentToken() == "<!--")
    {
        parser.NextToken();    // skip <!--

        string data;
        while (parser.CurrentToken() != "-->")
        {
            data += parser.Character();
            ++parser;
        }
        parser.NextToken();

        return new XmlComment(ownerDocument, parentNode, data);
    }
    // </...
    else if (parser.CurrentToken() == "</")
    {
        parser.NextToken();    // skip </
        if (parentNode != 0)
        {
            if (parser.CurrentToken() != parentNode->LocalName())
                throw string("Wrong closing tag found: ") + parser.CurrentToken() + string(" instead of ") + parentNode->LocalName();
            else
            {
                while (parser.CurrentToken() != ">")
                    parser.NextToken();
                parser.NextToken(); // skip >
                return 0;
            }
        }
        else
            throw string("Closing tag found, were none was needed: " + parser.CurrentToken());
    }
    // <...> & <.../>
    else if (parser.CurrentToken() == "<")
    {
        // Load all tokens within the tag, so they can be transformed into attributes later
        vector<string> tokens;
        string token = parser.NextToken();   // skip <
        while (token != ">" && token != "/>")
        {
            tokens.push_back(token);
            token = parser.NextToken();
        }
        parser.NextToken();   // skip >

        // Are there any tokens found before the tag was closed?
        if (tokens.size() >= 1)
        {
            XmlNode* node = new XmlNode(ownerDocument, parentNode, tokens[0]);
            if (token == ">")
            {
                XmlNode* child = XmlNode::_LoadNode(ownerDocument, node, parser);
                while (child != 0)
                {
                    node->_childNodes.push_back(child);
                    child = XmlNode::_LoadNode(ownerDocument, node, parser);
                }
            }
            node->_attributes = XmlNode::_LoadAttributes(ownerDocument, node, tokens);
            return node;
        }
    }
    // regular text
    else
    {
        string data;
        while (parser.Character() != '<')
        {
            data += parser.Character();
            ++parser;
        }
        return new XmlText(ownerDocument, parentNode, data);
    }
    return 0;
}

XmlAttributeCollection XmlNode::_LoadAttributes(XmlDocument* ownerDocument, XmlNode* parentNode, const std::vector<std::string>& tokens)
{
    XmlAttributeCollection result;

    for (unsigned int i = 1; i < tokens.size() - 1; i++)
    {
        if (tokens[i] == "=")
        {
            string value = tokens[i+1];
            if (value[0] == '\"' && value[value.size()-1] == '\"')
                value = value.substr(1, value.size()-2);
            result.insert(make_pair(tokens[i-1], new XmlAttribute(ownerDocument, parentNode, tokens[i-1], value)));
        }
    }

    return result;
}

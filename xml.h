#ifndef XML_H
#define XML_H

#include <vector>
#include <map>
#include <set>
#include <string>

namespace common
{

namespace xml
{

namespace priv
{

class XmlParser
{
public:
    XmlParser(const std::string& xml);
    XmlParser(const XmlParser& other);
    virtual ~XmlParser();

    XmlParser& operator = (const XmlParser& other);
    XmlParser& operator += (int count);
    char operator ++ ();
    bool operator == (char c) const;

    bool HasToken();
    void SkipSpaces();
    std::string CurrentToken();
    std::string NextToken();
    char Character();

    const char* _data;
    const char* _cursor;
    int _size;
};

}

class XmlNode;
typedef std::vector<XmlNode*> XmlNodeList;

class XmlAttribute;
typedef std::map<std::string, XmlAttribute*> XmlAttributeCollection;

class XmlDocument;

class XmlNode
{
public:
    XmlNode(XmlDocument* ownerDocument, XmlNode* parentNode, const std::string& localname);
    virtual ~XmlNode();

    static XmlNodeList LoadXml(XmlDocument* ownerDocument, const std::string& xml);

    virtual std::string InnerText();
    virtual void InnerText(const std::string& innertext);
    virtual std::string InnerXml();
    virtual void InnerXml(const std::string& innerxml);
    virtual std::string OuterXml();

    XmlNodeList SelectNodes(const std::string& xpath);
    XmlNode* SelectSingleNode(const std::string& xpath);

    const std::string& LocalName() { return this->_localName; }
    XmlDocument* OwnerDocument() { return this->_ownerDocument; }

    XmlAttributeCollection& Attributes() { return this->_attributes; }
    XmlNodeList& ChildNodes() { return this->_childNodes; }

protected:
    XmlDocument* _ownerDocument;
    XmlNode* _parentNode;
    std::string _localName;
    XmlAttributeCollection _attributes;
    XmlNodeList _childNodes;

private:
    void ClearAttributes();
    void ClearChildNodes();

protected:
    static XmlNode* _LoadNode(XmlDocument* ownerDocument, XmlNode* parentNode, priv::XmlParser& parser);
    static XmlAttributeCollection _LoadAttributes(XmlDocument* ownerDocument, XmlNode* parentNode, const std::vector<std::string>& tokens);
};

class XmlDeclaration : public XmlNode
{
public:
    XmlDeclaration(XmlDocument* ownerDocument, const std::vector<std::string>& tokens);
    virtual ~XmlDeclaration();

    virtual std::string OuterXml();
};

class XmlCharacterData : public XmlNode
{
public:
    XmlCharacterData(XmlDocument* ownerDocument, XmlNode* parentNode, const std::string& data);
    virtual ~XmlCharacterData();

    virtual std::string InnerText() { return this->_data; }
    virtual void InnerText(const std::string& data);

    virtual std::string OuterXml();

protected:
    std::string _data;

};

class XmlText : public XmlCharacterData
{
public:
    XmlText(XmlDocument* ownerDocument, XmlNode* parentNode, const std::string& text);
    virtual ~XmlText();

    virtual std::string OuterXml();
};

class XmlComment : public XmlNode
{
public:
    XmlComment(XmlDocument* ownerDocument, XmlNode* parentNode, const std::string& comment);
    virtual ~XmlComment();

    const std::string& Comment() const { return this->_comment; }
    void Comment(const std::string& comment);

    virtual std::string OuterXml();

private:
    std::string _comment;
};

class XmlAttribute : public XmlNode
{
public:
    XmlAttribute(XmlDocument* ownerDocument, XmlNode* parentNode, const std::string& key, const std::string& value);
    virtual ~XmlAttribute();

    virtual const std::string& Key() const { return this->_key; }

    virtual const std::string& Value() const { return this->_value; }
    virtual void Value(const std::string& value);

private:
    std::string _key;
    std::string _value;
};

class XmlDocument
{
public:
    XmlDocument();
    virtual ~XmlDocument();

    bool Load(const std::string& filename);
    bool LoadXml(const std::string& xml);

    XmlNode* DocumentElement() { return this->_documentElement; }

    XmlNodeList SelectNodes(const std::string& xpath);
    XmlNode* SelectSingleNode(const std::string& xpath);

public:
    XmlNode* _declaration;
    XmlNode* _documentElement;
    std::set<XmlNode*> _nodes;

};

}   // common

}   // xml

#endif // XML_H


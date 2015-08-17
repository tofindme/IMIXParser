#pragma once

#include "../defines.h"

#define TAGS	5

class IMIXDocument;

class IMIXAssort
{
public:
	IMIXAssort(void);
	~IMIXAssort(void);

	//加载IMIX消息文件
	void loadFile(const char *fileName);
	//遍历整个xml文件
	void iteratorDoc(void);
	void iteratorMsgs(rapidxml::xml_node<> *msgs);

	//输出到文件
	void appToFile(const char *fileName);
	void adminToFile(const char *fileName);

private:
	
	//构建一个header/trailer/messages/componenst/fields结构
	void appConstru(void);
	void adminConstru(void);

private:
	rapidxml::xml_document<> *m_doc;
	rapidxml::xml_node<> *m_root;
	rapidxml::file<> *m_file;

	IMIXDocument *m_app;	//存放app消息结构的doc
	IMIXDocument *m_admin;	//存放admin消息结构的doc
};


class IMIXDocument
{
public:
	IMIXDocument();
	~IMIXDocument();
	//初始化声明部分
	void IMIXDocument::initDeclar(rapidxml::xml_node<> *node);

	//给doc创建一个header、trailer、messages、components结构
	void initEelements(rapidxml::xml_node<> *node);	

	//遍历并追加header里面的标签
	void appendHeader(rapidxml::xml_node<> *head);

	//遍历并追加trailer里面的标签
	void appendTrailer(rapidxml::xml_node<> *trail);

	//遍历messages标签并添加
	void appendMsgs(rapidxml::xml_node<> *msgs);

	//设置指向外部的xml结构
	void setExtRoot(rapidxml::xml_node<> *rt);

	//内容输出到文件
	void outFile(const char *fileName);

	

private:
	//追加结点的属性
	void appendAttr(rapidxml::xml_attribute<> *&attr, rapidxml::xml_node<> *&node);

	//追加名称为参数的component结点
	void appendCmpnt(const std::string name);

	//在外部的文档里查找为参数的component
	rapidxml::xml_node<> * findCmpnt(const std::string name);

	//追加component下边的group结点
	void appendGroup(rapidxml::xml_node<> *cmpnt, rapidxml::xml_node<> *&grp);

	//追加为参数名称的field
	void appendField(const std::string name);

	//在外部的文档里查找为参数名称的field
	rapidxml::xml_node<> * findField(const std::string name);

	//追加field下面的value
	void appendFldValue(rapidxml::xml_node<> *fld,rapidxml::xml_node<> *node);

	//field是否已经存在
	bool existFld(std::string name);

	//component是否已经存在
	bool existCmpnt(std::string name);

private:
	rapidxml::xml_document<> *m_doc;	//构建的新消息内存结构
	rapidxml::xml_node<> *m_root;		//指向有子结点的第一个结点

	rapidxml::xml_node<> *extRoot;	//指向外部doc的最上一层父结点
};



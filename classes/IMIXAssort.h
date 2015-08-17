#pragma once

#include "../defines.h"

#define TAGS	5

class IMIXDocument;

class IMIXAssort
{
public:
	IMIXAssort(void);
	~IMIXAssort(void);

	//����IMIX��Ϣ�ļ�
	void loadFile(const char *fileName);
	//��������xml�ļ�
	void iteratorDoc(void);
	void iteratorMsgs(rapidxml::xml_node<> *msgs);

	//������ļ�
	void appToFile(const char *fileName);
	void adminToFile(const char *fileName);

private:
	
	//����һ��header/trailer/messages/componenst/fields�ṹ
	void appConstru(void);
	void adminConstru(void);

private:
	rapidxml::xml_document<> *m_doc;
	rapidxml::xml_node<> *m_root;
	rapidxml::file<> *m_file;

	IMIXDocument *m_app;	//���app��Ϣ�ṹ��doc
	IMIXDocument *m_admin;	//���admin��Ϣ�ṹ��doc
};


class IMIXDocument
{
public:
	IMIXDocument();
	~IMIXDocument();
	//��ʼ����������
	void IMIXDocument::initDeclar(rapidxml::xml_node<> *node);

	//��doc����һ��header��trailer��messages��components�ṹ
	void initEelements(rapidxml::xml_node<> *node);	

	//������׷��header����ı�ǩ
	void appendHeader(rapidxml::xml_node<> *head);

	//������׷��trailer����ı�ǩ
	void appendTrailer(rapidxml::xml_node<> *trail);

	//����messages��ǩ�����
	void appendMsgs(rapidxml::xml_node<> *msgs);

	//����ָ���ⲿ��xml�ṹ
	void setExtRoot(rapidxml::xml_node<> *rt);

	//����������ļ�
	void outFile(const char *fileName);

	

private:
	//׷�ӽ�������
	void appendAttr(rapidxml::xml_attribute<> *&attr, rapidxml::xml_node<> *&node);

	//׷������Ϊ������component���
	void appendCmpnt(const std::string name);

	//���ⲿ���ĵ������Ϊ������component
	rapidxml::xml_node<> * findCmpnt(const std::string name);

	//׷��component�±ߵ�group���
	void appendGroup(rapidxml::xml_node<> *cmpnt, rapidxml::xml_node<> *&grp);

	//׷��Ϊ�������Ƶ�field
	void appendField(const std::string name);

	//���ⲿ���ĵ������Ϊ�������Ƶ�field
	rapidxml::xml_node<> * findField(const std::string name);

	//׷��field�����value
	void appendFldValue(rapidxml::xml_node<> *fld,rapidxml::xml_node<> *node);

	//field�Ƿ��Ѿ�����
	bool existFld(std::string name);

	//component�Ƿ��Ѿ�����
	bool existCmpnt(std::string name);

private:
	rapidxml::xml_document<> *m_doc;	//����������Ϣ�ڴ�ṹ
	rapidxml::xml_node<> *m_root;		//ָ�����ӽ��ĵ�һ�����

	rapidxml::xml_node<> *extRoot;	//ָ���ⲿdoc������һ�㸸���
};



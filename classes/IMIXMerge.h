#pragma  once
#include "../defines.h"

#define LOG_NAME	"imix.log"
 

class IMIXMerge
{
public:
	IMIXMerge();
	virtual ~IMIXMerge();

	enum TYPE
	{
		HEADER = 0,
		MESSAGES,
		TRAILER,
		COMPONENTS,
		FIELDS
	};
  
	//��ȡ��Ϣ�ļ������ص��ڴ���
	void setOldXML(char *);
	void setNewXML(char *);
 
	//��xml����������ļ���
	void outFile(const char *fileName);
	int beginCompare();

private: 
	//�Ƚ���header/trailer��ǩ
	int singleCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new,TYPE type = TYPE(0));

	//�Ƚ�fields��ǩ
	int fldCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new, TYPE tyep = TYPE(4));
	
	//�Ƚ�messages
	int msgCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new, TYPE type = TYPE(1));

	//�Ƚ�components
	int cmpntCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new, TYPE type = TYPE(3));

	//���component�±ߵĽ��
	int appendCmpnt(rapidxml::xml_node<> *des, rapidxml::xml_node<> *src);

	//�������group�������±ߵĽ��
	int appendGroup(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src, TYPE type = FIELDS);

	//���field�±ߵ�value
	int appendFldValue(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src);

	//��ӽڵ������
	void appendAttr(rapidxml::xml_node<> *&des, rapidxml::xml_attribute<> *&attr);

private:
	rapidxml::file<> *m_fileOld;		//��ȡ��Ϣ���͵��ļ��¾ɰ汾��һ��
	rapidxml::file<> *m_fileNew;		
	rapidxml::xml_document<> *m_oldDoc;	//������Ϣ�ļ���ŵ�������߽ṹ
	rapidxml::xml_document<> *m_newDoc;
	rapidxml::xml_node<> * m_rootOld;	//ָ����ϢXML��ָ��
	rapidxml::xml_node<> * m_rootNew;

	std::ofstream	m_strLog;		//��¼���Բ�ͬ���ļ������
};

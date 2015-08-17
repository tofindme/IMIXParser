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
  
	//读取消息文件并加载到内存里
	void setOldXML(char *);
	void setNewXML(char *);
 
	//把xml内容输出到文件中
	void outFile(const char *fileName);
	int beginCompare();

private: 
	//比较像header/trailer标签
	int singleCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new,TYPE type = TYPE(0));

	//比较fields标签
	int fldCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new, TYPE tyep = TYPE(4));
	
	//比较messages
	int msgCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new, TYPE type = TYPE(1));

	//比较components
	int cmpntCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new, TYPE type = TYPE(3));

	//添加component下边的结点
	int appendCmpnt(rapidxml::xml_node<> *des, rapidxml::xml_node<> *src);

	//添加类似group有属性下边的结点
	int appendGroup(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src, TYPE type = FIELDS);

	//添加field下边的value
	int appendFldValue(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src);

	//添加节点的属性
	void appendAttr(rapidxml::xml_node<> *&des, rapidxml::xml_attribute<> *&attr);

private:
	rapidxml::file<> *m_fileOld;		//读取消息类型的文件新旧版本各一份
	rapidxml::file<> *m_fileNew;		
	rapidxml::xml_document<> *m_oldDoc;	//解析消息文件后放到内在里边结构
	rapidxml::xml_document<> *m_newDoc;
	rapidxml::xml_node<> * m_rootOld;	//指向消息XML的指针
	rapidxml::xml_node<> * m_rootNew;

	std::ofstream	m_strLog;		//记录属性不同的文件输出流
};

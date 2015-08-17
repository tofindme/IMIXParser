#pragma  once
#include "../defines.h"



#define IMIX_SUCCESS		0		//成功
#define IMIX_HEADER_ERR		1		//消息头前三顺序不对8 9 35
#define IMIX_TRAILER_ERR	2		//消息尾最后不是域号10
#define IMIX_VALUE_ERR		3		//域的值不存在
#define IMIX_REQ_ERR		4		//域的必须域缺少
#define IMIX_POSI_ERR		5		//消息的位置错乱
#define IMIX_REPEAT_ERR		6		//消息的域重复
#define IMIX_NOT_FIND_ERR   7		//未找到
#define IMIX_DATA_LOSE		8		//数据有丢失
#define IMIX_GRP_FIRST_ERR		9		 
#define IMIX_GRP_REPEAT_ERR	10 
#define IMIX_GRP_COUNT_ERR	11

class IMIXCheck
{ 
	typedef map<int,std::string> FldKeyNo;
	typedef map<std::string,int> FldKeyName;
	typedef set<std::string> Value;//存放field对应的值
	typedef map<int,Value> FldToValue;
	typedef map<int,bool>  FldIsRequired;
	typedef set<int>	Fields;	
	typedef map<std::string,Fields> MsgFlds;
	typedef map<std::pair<int,std::string>,std::string> ValueToName;
	typedef map<std::pair<std::string,int>,std::pair<int,IMIXCheck*>> Group;


public:
	IMIXCheck();
	virtual ~IMIXCheck();

	//消息的文件名称
	void setXMLData(char *);

	//读取数据，并存放到数据结构里
	void readData(void);

	//判断消息的合法性
	int validate(const char *buf, int len);

	//获取消息下一个域的num及value
	char * getValue2Next(const char *str, std::string &value);

	//判断是否需要检测的域
	bool shouldCheck(int);
	
	//设置不需要检测的域
	void setExtraFld(int);

	int getExtraFld(void);

private:
	//从buf里解析消息头
	int parseHeader(const char *buf, int &headerLen);	

	//解析消息头中的group
	int parseHeaderGroup(int field,std::string value, const char *buf,int &skip);

	//判断消息头的顺序是否为8=...9=...35=...
	bool headerIsOrder(const char *buf);

	//是否是消息头中的域
	bool isHeaderFld(int field);

	//是否域有枚举值
	bool hasValue(int);

	//从buf里解析消息尾
	int parseTrailer(const char *buf, int &trailerLen);

	//是还是是消息尾中的域
	bool isTrailerFld(int);

	//消息尾是否是以10=...结束
	bool parseLastFld(const char*);

	//从buf里解析消息体
	int parseMsg(const char *begin, int &msgLen);

	//解析消息中的重复组
	int parseGroup(int ,std::string,const char *,int&);

	//是否是一个重复组
	bool isGroupFld(std::string, int field);

	//判断域是否属于重复组
	bool fldInGroup(std::string,int);

	//寻找消息里边的域
	bool findFldInMsg(int);

	//在field下面寻找value值
	bool findValueInFld(int,std::string);


	int addXMLComponent(const std::string &name, bool isRequired, std::string msgType, IMIXCheck &dd);
	//从xml里面读取重复组
	int addXMLGroup(rapidxml::xml_node<> *groupNode, std::string msgType, IMIXCheck &dd);

	//从map里查找域号
	int lookupXMLFldNumber(rapidxml::xml_node<> *node);
	int lookupXMLFldNumber(const string &name);

	//是否包含field域
	bool lookupFld(int);

	//放到field的map里
	void addFlds(int num);

	//存放消息中所有消息类型
	void addFldName(int num,std::string name);

	//放到field存放value的集合里
	void addFldValue(int num, std::string value);

	void addValueName(int field, const std::string& value, const std::string& name);
	//存放msgs域重复组下的域也用此函数
	int addMsgFlds(std::string msgType, int num, bool);

	//新增重复组到group的map里
	void addGroup(std::string msg,int field,int first, IMIXCheck *&dd);

	//存放header的域
	void addHeaderFlds(int, bool);

	//存放trailer里的域
	void addTrailerFlds(int, bool);

private:
	rapidxml::file<> *m_file;
	rapidxml::xml_document<> *m_doc;
	rapidxml::xml_node<> * m_root;

	int extraFld;						//不需要验证的域

	FldKeyNo m_mFldKeyNo;				//存放field 域号作为key
	FldKeyName m_mFldKeyName;			//存放field 域名作为key
	FldToValue m_mValueToFld;			//存放field对应的value值
	FldIsRequired m_mHeadFlds;			//存放header下面的field域 域号作为key 是否必须作为value
	FldIsRequired m_mTrailFlds;			//存放trailer下面的field域 域号作为key 是否必须作为value
	MsgFlds		m_mMsgFlds;				//存放msg下面的fields
	FldIsRequired		m_mMsgFldsIsBool;		//存放msg下面是必须项的field
	Fields		m_sFlds;
	ValueToName m_mValueToName;			//
	Group		m_mGroup;				//重复组
	std::string msgType;
};

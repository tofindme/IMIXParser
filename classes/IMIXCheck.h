#pragma  once
#include "../defines.h"



#define IMIX_SUCCESS		0		//�ɹ�
#define IMIX_HEADER_ERR		1		//��Ϣͷǰ��˳�򲻶�8 9 35
#define IMIX_TRAILER_ERR	2		//��Ϣβ��������10
#define IMIX_VALUE_ERR		3		//���ֵ������
#define IMIX_REQ_ERR		4		//��ı�����ȱ��
#define IMIX_POSI_ERR		5		//��Ϣ��λ�ô���
#define IMIX_REPEAT_ERR		6		//��Ϣ�����ظ�
#define IMIX_NOT_FIND_ERR   7		//δ�ҵ�
#define IMIX_DATA_LOSE		8		//�����ж�ʧ
#define IMIX_GRP_FIRST_ERR		9		 
#define IMIX_GRP_REPEAT_ERR	10 
#define IMIX_GRP_COUNT_ERR	11

class IMIXCheck
{ 
	typedef map<int,std::string> FldKeyNo;
	typedef map<std::string,int> FldKeyName;
	typedef set<std::string> Value;//���field��Ӧ��ֵ
	typedef map<int,Value> FldToValue;
	typedef map<int,bool>  FldIsRequired;
	typedef set<int>	Fields;	
	typedef map<std::string,Fields> MsgFlds;
	typedef map<std::pair<int,std::string>,std::string> ValueToName;
	typedef map<std::pair<std::string,int>,std::pair<int,IMIXCheck*>> Group;


public:
	IMIXCheck();
	virtual ~IMIXCheck();

	//��Ϣ���ļ�����
	void setXMLData(char *);

	//��ȡ���ݣ�����ŵ����ݽṹ��
	void readData(void);

	//�ж���Ϣ�ĺϷ���
	int validate(const char *buf, int len);

	//��ȡ��Ϣ��һ�����num��value
	char * getValue2Next(const char *str, std::string &value);

	//�ж��Ƿ���Ҫ������
	bool shouldCheck(int);
	
	//���ò���Ҫ������
	void setExtraFld(int);

	int getExtraFld(void);

private:
	//��buf�������Ϣͷ
	int parseHeader(const char *buf, int &headerLen);	

	//������Ϣͷ�е�group
	int parseHeaderGroup(int field,std::string value, const char *buf,int &skip);

	//�ж���Ϣͷ��˳���Ƿ�Ϊ8=...9=...35=...
	bool headerIsOrder(const char *buf);

	//�Ƿ�����Ϣͷ�е���
	bool isHeaderFld(int field);

	//�Ƿ�����ö��ֵ
	bool hasValue(int);

	//��buf�������Ϣβ
	int parseTrailer(const char *buf, int &trailerLen);

	//�ǻ�������Ϣβ�е���
	bool isTrailerFld(int);

	//��Ϣβ�Ƿ�����10=...����
	bool parseLastFld(const char*);

	//��buf�������Ϣ��
	int parseMsg(const char *begin, int &msgLen);

	//������Ϣ�е��ظ���
	int parseGroup(int ,std::string,const char *,int&);

	//�Ƿ���һ���ظ���
	bool isGroupFld(std::string, int field);

	//�ж����Ƿ������ظ���
	bool fldInGroup(std::string,int);

	//Ѱ����Ϣ��ߵ���
	bool findFldInMsg(int);

	//��field����Ѱ��valueֵ
	bool findValueInFld(int,std::string);


	int addXMLComponent(const std::string &name, bool isRequired, std::string msgType, IMIXCheck &dd);
	//��xml�����ȡ�ظ���
	int addXMLGroup(rapidxml::xml_node<> *groupNode, std::string msgType, IMIXCheck &dd);

	//��map��������
	int lookupXMLFldNumber(rapidxml::xml_node<> *node);
	int lookupXMLFldNumber(const string &name);

	//�Ƿ����field��
	bool lookupFld(int);

	//�ŵ�field��map��
	void addFlds(int num);

	//�����Ϣ��������Ϣ����
	void addFldName(int num,std::string name);

	//�ŵ�field���value�ļ�����
	void addFldValue(int num, std::string value);

	void addValueName(int field, const std::string& value, const std::string& name);
	//���msgs���ظ����µ���Ҳ�ô˺���
	int addMsgFlds(std::string msgType, int num, bool);

	//�����ظ��鵽group��map��
	void addGroup(std::string msg,int field,int first, IMIXCheck *&dd);

	//���header����
	void addHeaderFlds(int, bool);

	//���trailer�����
	void addTrailerFlds(int, bool);

private:
	rapidxml::file<> *m_file;
	rapidxml::xml_document<> *m_doc;
	rapidxml::xml_node<> * m_root;

	int extraFld;						//����Ҫ��֤����

	FldKeyNo m_mFldKeyNo;				//���field �����Ϊkey
	FldKeyName m_mFldKeyName;			//���field ������Ϊkey
	FldToValue m_mValueToFld;			//���field��Ӧ��valueֵ
	FldIsRequired m_mHeadFlds;			//���header�����field�� �����Ϊkey �Ƿ������Ϊvalue
	FldIsRequired m_mTrailFlds;			//���trailer�����field�� �����Ϊkey �Ƿ������Ϊvalue
	MsgFlds		m_mMsgFlds;				//���msg�����fields
	FldIsRequired		m_mMsgFldsIsBool;		//���msg�����Ǳ������field
	Fields		m_sFlds;
	ValueToName m_mValueToName;			//
	Group		m_mGroup;				//�ظ���
	std::string msgType;
};

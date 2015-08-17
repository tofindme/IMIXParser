#include "IMIXMerge.h"

IMIXMerge::IMIXMerge():m_rootOld(NULL),m_rootNew(NULL),m_fileOld(NULL),m_fileNew(NULL),m_oldDoc(NULL),m_newDoc(NULL)
{
	m_strLog.open(LOG_NAME,ios_base::out | ios_base::_Noreplace);
}

IMIXMerge::~IMIXMerge()
{
	SAFE_FREE(m_fileOld);
	SAFE_FREE(m_fileNew);
	SAFE_FREE(m_oldDoc);
	SAFE_FREE(m_newDoc);
}

/*
��ȡ�°汾��Ϣ
@fileName	�ļ�����
*/
void IMIXMerge::setOldXML(char *fileName)
{
	assert(fileName != NULL);

	SAFE_FREE(m_fileOld);
	SAFE_FREE(m_oldDoc);


	m_fileOld = new rapidxml::file<>(fileName);
	m_oldDoc = new rapidxml::xml_document<>();

	try
	{
		this->m_oldDoc->parse<0x20>(m_fileOld->data());
		this->m_rootOld = this->m_oldDoc->last_node();
	}
	catch (rapidxml::parse_error  e)
	{
		char *str = (char*)malloc(strlen(e.what())+601);
		memset(str,0,strlen(str));
		strcat(str,e.what());
		strcat(str," at ");
		strncat(str,e.where<char>(),600);
		delete m_oldDoc;m_oldDoc = NULL;
		//AfxMessageBox(str,MB_OK | MB_ICONERROR);
	}
	catch (std::exception e)
	{

	}

}

/*
��ȡ�°汾��Ϣ
@fileName	�ļ�����
*/
void IMIXMerge::setNewXML(char *fileName)
{
	assert(fileName != NULL);

	SAFE_FREE(m_fileNew);
	SAFE_FREE(m_newDoc);


	m_fileNew = new rapidxml::file<>(fileName);
	m_newDoc = new rapidxml::xml_document<>();

	try
	{
		this->m_newDoc->parse<0x20>(m_fileNew->data());//0x20���������������
		this->m_rootNew = this->m_newDoc->last_node();
	}
	catch (rapidxml::parse_error e)
	{ 
		char *str = (char*)malloc(strlen(e.what())+601);
		memset(str,0,strlen(str));
		strcat(str,e.what());
		strcat(str," at ");
		strncat(str, e.where<char>(),600); 
		delete m_newDoc;m_newDoc = NULL;
		//AfxMessageBox(str,MB_OK | MB_ICONERROR);
	}
	catch (exception e)
	{
	}
}

/*
�ṩ�ӿڿ�ʼ�Ƚ�
˳��Ϊ��Ϣ��ǩ��˳��
1.header
2.trailer
3.messages
4.components
5.fields
*/
int IMIXMerge::beginCompare()
{
	assert(m_rootOld != NULL && m_rootNew != NULL);
	//�Ƚ�header���
	singleCompare(m_rootOld->first_node(TAG_HEADER),m_rootNew->first_node(TAG_HEADER),HEADER);
	//�Ƚ�trailer���
	singleCompare(m_rootOld->first_node(TAG_TRAILER),m_rootNew->first_node(TAG_TRAILER),TRAILER);
	//�Ƚ�messages���
	msgCompare(m_rootOld->first_node(TAG_MESSAGES),m_rootNew->first_node(TAG_MESSAGES),MESSAGES);
	//�Ƚ�components���
	cmpntCompare(m_rootOld->first_node(TAG_COMPONENTS),m_rootNew->first_node(TAG_COMPONENTS),COMPONENTS);
	//�Ƚ�fields���
	fldCompare(m_rootOld->first_node(TAG_FIELDS),m_rootNew->first_node(TAG_FIELDS),FIELDS);

	return 1;
}

//�ϲ���ָ�������ָ���ļ�
void IMIXMerge::outFile(const char *fileName)
{
	ofstream outXML;
	if(fileName == NULL)
	{
		outXML.open("merge.xml",ios_base::out | ios_base::trunc);
	}
	else
	{
		outXML.open(fileName,ios_base::out | ios_base::trunc);
	}

	outXML<<*m_newDoc;
}

/* �Ƚ��������±�ǩ
	<name>
		<subtag attr = .../>
		<subtag attr = .../>
	</name>
@x_old	ָ��ɰ汾��Ϣ�е���Ҫ�Ƚϵĵ�һ����� ��<header>��ǩ�µĵ�һ���ӽ��
@x_new	ָ���°汾��Ϣ�е���Ҫ�Ƚϵĵ�һ�����
@type   �Ƚϵı�ǩ����Ĭ��Ϊheader
*/
int IMIXMerge::singleCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new,TYPE type)
{
	rapidxml::xml_node<>* subOld = x_old->first_node();
	rapidxml::xml_node<>* subNew = x_new->first_node();
	rapidxml::xml_node<>* newTemp = subNew;
	
	//�Ծ���Ϣ�����Ϊ׼������Ϣ����ȥѰ��
	while(subOld != NULL)
	{
		std::string o_name,o_required,o_value;

		if(type == HEADER || type == TRAILER || type == MESSAGES || type == COMPONENTS)
		{
			o_name = subOld->first_attribute(ATTR_NAME)->value();
			o_required = subOld->first_attribute(ATTR_REQUIR)->value();
		}
		else if(type == FIELDS)
		{
			o_value = subOld->first_attribute("enum")->value();
		}
		//���µ���Ϣ����ȥ������ǰ����µ����н��
		while(newTemp != NULL)
		{
			std::string n_name,n_required,n_value;

			if(type == HEADER || type == TRAILER || type == MESSAGES || type == COMPONENTS)
			{
				n_name = newTemp->first_attribute(ATTR_NAME)->value();
				n_required = newTemp->first_attribute(ATTR_REQUIR)->value();
				if(o_name == n_name)
				{
					if(o_required == n_required)
					{
						break;
					}
					else
					{
						m_strLog<<"required���Բ�һ�� name = "<<o_name<<"  �ɰ汾�ı������� required = "<<o_required<<"   �ɰ汾�ı������� required = "<<n_required<<std::endl;
						//���Ƿ���벻ƥ����Ծɰ汾��Ϊ׼
						//newTemp->first_attribute(ATTR_REQUIR)->value(subOld->first_attribute("required")->value());
						break;
					}
				}
			}
			else if(type == FIELDS)
			{
				n_value = newTemp->first_attribute(ATTR_ENUM)->value();
				if(o_value == n_value)
				{
					break;
				}				
			}
			newTemp = newTemp->next_sibling();
		}
		//û�ҵ����
		if(NULL == newTemp)
		{
			//��ȱ�ٵĽڽ�ϵ��µİ汾��doc�����
			rapidxml::xml_node<>* child = NULL;
			rapidxml::xml_attribute<> *attr = NULL;
			std::string req,tagName;
			//����һ�����
			child = m_newDoc->allocate_node(rapidxml::node_element,subOld->name());
			attr = subOld->first_attribute();

			tagName = subOld->name();
			if(subOld->first_attribute(ATTR_REQUIR))
			{
				req = subOld->first_attribute(ATTR_REQUIR)->value();
			}
				
			appendAttr(child,attr);
			
			//����component����Ľ����Ϊ��������������ǰ�߲���
			if((req == "Y" || req == "y") && type == COMPONENTS)
			{
				x_new->prepend_node(child);
			}//����������׷��
			else
			{
				x_new->append_node(child);
			}
			
		}

		newTemp = subNew;//��ͷ��ʼ����
		subOld = subOld->next_sibling();
	}

	return 1;
}

/*
�Ƚ�fields������ı�ǩ
@x_old	ָ��ɰ汾��Ϣ�е�fields�ĵ�һ�����
@x_new	ָ���°汾��Ϣ�е�fields�ĵ�һ�����
@type   �Ƚϵı�ǩ����Ĭ��Ϊfields
*/

int IMIXMerge::fldCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new, TYPE type)
{
	rapidxml::xml_node<>* subOld = x_old->first_node();
	rapidxml::xml_node<>* subNew = x_new->first_node();
	rapidxml::xml_node<>* newTemp = subNew;

	while(subOld != NULL)
	{
		std::string o_name,o_type;
		o_name = subOld->first_attribute(ATTR_NAME)->value();
		o_type = subOld->first_attribute(ATTR_TYPE)->value();

		while(newTemp != NULL)
		{
			std::string n_name,n_type;
			n_name = newTemp->first_attribute(ATTR_NAME)->value();
			n_type = newTemp->first_attribute(ATTR_TYPE)->value();

			if(o_name == n_name)
			{
				if(o_type == n_type)
				{
					break;
				}
				else
				{
					//��ͬ�����Բ�ͬ
					m_strLog<<"���type���Բ�һ�� name = "<<o_name<<"  �ɰ汾�� type = "<<o_type<<"  �°汾�� type = "<<n_type<<std::endl;

					//newTemp->first_attribute("type")->value(subOld->first_attribute("type")->value());
					//std::cout<<"type not compatable!"<<std::endl;
					break;
				}
			}
			newTemp = newTemp->next_sibling();
		}
		if(NULL == newTemp)
		{
			rapidxml::xml_node<> *fld = NULL;
			appendGroup(fld,subOld,type);
			x_new->append_node(fld);
		}
		else
		{
			rapidxml::xml_node<>* sub2Old = subOld->first_node();
			rapidxml::xml_node<>* sub2New = newTemp->first_node();

			if(sub2Old != NULL && sub2New != NULL)
			{
				singleCompare(subOld,newTemp,type);
			}
			else if(sub2Old != NULL && sub2New == NULL)
			{
				appendFldValue(newTemp, sub2Old);
			}
		}
		newTemp = subNew;
		subOld = subOld->next_sibling();
	}
	return 1;
}

/*
�Ƚ�messages��ǩ
@x_old	ָ��ɰ汾��Ϣ�е�messages�ĵ�һ�����
@x_new	ָ���°汾��Ϣ�е�messages�ĵ�һ�����
@type   �Ƚϵı�ǩ����Ĭ��Ϊmessages
*/
int IMIXMerge::msgCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new,TYPE type)
{
	rapidxml::xml_node<> *subOld = x_old->first_node();
	rapidxml::xml_node<> *subNew = x_new->first_node();
	rapidxml::xml_node<> *subTemp = subNew;

	while(subOld != NULL)
	{
		std::string o_name,o_type,o_cat,o_required;
		o_name = subOld->first_attribute(ATTR_NAME)->value();
		o_type = subOld->first_attribute(ATTR_MSGTYPE)->value();
		o_cat = subOld->first_attribute(ATTR_MSGCAT)->value();

		while(subTemp != NULL)
		{
			std::string n_name,n_type,n_cat,n_required;
			n_name = subTemp->first_attribute(ATTR_NAME)->value();
			n_type = subTemp->first_attribute(ATTR_MSGTYPE)->value();
			n_cat = subTemp->first_attribute(ATTR_MSGCAT)->value();

			if(o_name == n_name)
			{
				if(o_type != n_type)
				{
					m_strLog<<"��Ϣ��ǩ��msgtype���Բ�һ�� name = "<<o_name<<"  �ɰ汾������Ϊ: "<<o_type<<"  �ɰ汾������Ϊ: "<<n_type<<std::endl;
				}

				if(o_cat != n_cat)
				{
					m_strLog<<"��Ϣ��ǩ��msgcat���Բ�һ�� name = "<<o_name<<"  �ɰ汾������Ϊ: "<<o_cat<<"  �ɰ汾������Ϊ: "<<n_cat<<std::endl;
				}
				break;
			}
			subTemp = subTemp->next_sibling();
		}
		if(NULL == subTemp)
		{
			rapidxml::xml_node<> *msg = NULL;
			appendGroup(msg, subOld,type);
			x_new->append_node(msg);
		}
		else
		{
			singleCompare(subOld,subTemp,type);
		}
		subTemp = subNew;
		subOld = subOld->next_sibling();
	}
	return 1;
}

/*
�Ƚ�component��ǩ
@x_old	ָ��ɰ汾��Ϣ�е�components�ĵ�һ�����
@x_new	ָ���°汾��Ϣ�е�components�ĵ�һ�����
@type   �Ƚϵı�ǩ����Ĭ��Ϊcomonents
*/
int IMIXMerge::cmpntCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new,TYPE type)
{
	rapidxml::xml_node<> *subOld = x_old->first_node();
	rapidxml::xml_node<> *subNew = x_new->first_node();
	rapidxml::xml_node<> *subTemp = subNew;

	while(subOld != NULL)
	{
		std::string o_name;
		o_name = subOld->first_attribute(ATTR_NAME)->value();
		while(subTemp != NULL)
		{
			std::string n_name,n_type,n_cat;
			n_name = subTemp->first_attribute(ATTR_NAME)->value();
			if(o_name == n_name)
			{
				break;
			}
			subTemp = subTemp->next_sibling();
		}
		if(NULL == subTemp)
		{
			//�ڷ���compnentû�ҵ�֮��Ѷ�Ӧ��compnent����Ľ��׷�ӽ�ȥ
			appendCmpnt(x_new,subOld);

			//std::cout<<o_name<<"	not exists"<<std::endl;
		}
		else
		{
			rapidxml::xml_node<>* sub2Old = subOld->first_node();
			rapidxml::xml_node<>* sub2New = subTemp->first_node();
			if(!memcmp(sub2Old->name(),TAG_GROUP,strlen(TAG_GROUP)) 
				&& sub2New != NULL && !memcmp(sub2New->name(),TAG_GROUP,strlen(TAG_GROUP))
				&& !memcmp(sub2Old->name(),sub2New->name(),strlen(TAG_GROUP)))
			{
				singleCompare(sub2Old,sub2New,type);
			}
			else if(!memcmp(sub2Old->name(),TAG_GROUP,strlen(TAG_GROUP)) 
				&& (sub2New == NULL || !memcmp(sub2Old->name(),sub2New->name(),strlen("group"))))
			{
				rapidxml::xml_node<> *group = NULL;
				appendGroup(group, sub2Old,type);
				subTemp->append_node(group);
				//std::cout<<"components �����"<<sub2Old->first_attribute()->value()<<"��groupû"<<std::endl;

			}
			else if(memcmp(sub2Old->name(),TAG_GROUP,strlen(TAG_GROUP)) && memcmp(sub2New->name(),TAG_GROUP,strlen(TAG_GROUP)))
			{
				singleCompare(subOld,subTemp,type);
			}
// 			subTemp = subNew;
// 			subOld = subOld->next_sibling();
// 			continue;
		}
		subTemp = subNew;
		subOld = subOld->next_sibling();
	}
	return 1;
}

/*
׷��һ��component���,�����µĽ��һͬ׷��
@des	��Ҫ׷�ӵ��ĵط�	
@src	ָ��ȱʧ��component
*/
int IMIXMerge::appendCmpnt(rapidxml::xml_node<> *des, rapidxml::xml_node<> *src)
{
	rapidxml::xml_node<> *a_cmpnt_nd = NULL;			
	rapidxml::xml_attribute<> *a_cmpnt_attr = NULL;

	//����һ���µ�component���
	a_cmpnt_nd = m_newDoc->allocate_node(rapidxml::node_element,src->name());		
	a_cmpnt_attr = src->first_attribute();

	//component������
	appendAttr(a_cmpnt_nd, a_cmpnt_attr);

	//��ȡҪ׷�ӵ�Դͷ����һ�����
	rapidxml::xml_node<> * subSrc = src->first_node();
	while(subSrc != NULL)			
	{
		rapidxml::xml_node<> *a_cmpnt_chd_nd = NULL;  //component������½��
		rapidxml::xml_attribute<> *attr = NULL;	  //�½�������
		std::string nodeName = subSrc->name();
		if (nodeName == "group")
		{
			//��group���ʱ�ѽ��һ��һ��׷��
			appendGroup(a_cmpnt_chd_nd, subSrc,COMPONENTS);
			a_cmpnt_nd->append_node(a_cmpnt_chd_nd);	//׷�ӵ��������component���

		}else
		{
			//����������㺬field or component
			a_cmpnt_chd_nd = m_newDoc->allocate_node(rapidxml::node_element,subSrc->name());
			attr = subSrc->first_attribute();
			
			//׷�ӽ�������
			appendAttr(a_cmpnt_chd_nd,attr);

			//׷�ӵ��������component���
			a_cmpnt_nd->append_node(a_cmpnt_chd_nd);	
		}
		subSrc = subSrc->next_sibling();
	}
	//�����½���component���׷�ӵ�Ŀ��λ��
	des->append_node(a_cmpnt_nd);
	return 1;
}

/************************************************************************/
/* �����������
	<name attr=...>
		<subname attr=..>
		</subname>
	</name>
	
	@des	��Ҫ��ӵĽ��
	@src	ָ��ȱʧ�Ľ��	
	@type	�ǽ��������Ա�������� 
	*/
	
/************************************************************************/
int IMIXMerge::appendGroup(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src,TYPE type)
{
	rapidxml::xml_attribute<> *attr = NULL;

	//�Ƚ�һ��group����㣬֮���ٰ��ӽ��ӽ���
	des = m_newDoc->allocate_node(rapidxml::node_element,src->name());
	attr = src->first_attribute();
	
	appendAttr(des,attr);

	rapidxml::xml_node<> *subGroup = src->first_node();

	while (subGroup != NULL)
	{
		rapidxml::xml_node<> *subNode = NULL; //ָ��group��ǩ�µ��ӽ��
		rapidxml::xml_attribute<> *subAttr = NULL;

		//�������
		subNode = m_newDoc->allocate_node(rapidxml::node_element,subGroup->name());
		subAttr = subGroup->first_attribute();

		appendAttr(subNode,subAttr);

		//�ٰѽ��ŵ������õ�group���
		des->append_node(subNode);	

		subGroup = subGroup->next_sibling();
	}
	return 1;
}

/*
׷��field�����value���
@des	��Ҫ׷�ӵ��ĵط�	
@src	ָ��ȱʧvalue����field
*/
int IMIXMerge::appendFldValue(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src)
{
	while(src != NULL)
	{
		//�������
		rapidxml::xml_node<> *valueNode = m_newDoc->allocate_node(rapidxml::node_element,src->name());
		rapidxml::xml_attribute<> *attr = src->first_attribute();

		//׷������
		appendAttr(valueNode,attr);

		//׷�ӽ��
		des->append_node(valueNode);

		src = src->next_sibling();
	}
	return 1;
}

/*
����ÿ����㲢�ŵ������
@des	��Ҫ������ԵĽ��
@attr	ָ�������������
*/
void IMIXMerge::appendAttr(rapidxml::xml_node<> *&des, rapidxml::xml_attribute<> *&attr)
{
	while(attr)
	{
		rapidxml::xml_attribute<> *att = m_newDoc->allocate_attribute(attr->name(),attr->value());
		des->append_attribute(att);

		attr = attr->next_attribute();
	}
}
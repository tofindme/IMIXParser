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
读取新版本消息
@fileName	文件名称
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
读取新版本消息
@fileName	文件名称
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
		this->m_newDoc->parse<0x20>(m_fileNew->data());//0x20代表解析声明部分
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
提供接口开始比较
顺序为消息标签的顺序
1.header
2.trailer
3.messages
4.components
5.fields
*/
int IMIXMerge::beginCompare()
{
	assert(m_rootOld != NULL && m_rootNew != NULL);
	//比较header结点
	singleCompare(m_rootOld->first_node(TAG_HEADER),m_rootNew->first_node(TAG_HEADER),HEADER);
	//比较trailer结点
	singleCompare(m_rootOld->first_node(TAG_TRAILER),m_rootNew->first_node(TAG_TRAILER),TRAILER);
	//比较messages结点
	msgCompare(m_rootOld->first_node(TAG_MESSAGES),m_rootNew->first_node(TAG_MESSAGES),MESSAGES);
	//比较components结点
	cmpntCompare(m_rootOld->first_node(TAG_COMPONENTS),m_rootNew->first_node(TAG_COMPONENTS),COMPONENTS);
	//比较fields结点
	fldCompare(m_rootOld->first_node(TAG_FIELDS),m_rootNew->first_node(TAG_FIELDS),FIELDS);

	return 1;
}

//合并合指定输出到指定文件
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

/* 比较类似以下标签
	<name>
		<subtag attr = .../>
		<subtag attr = .../>
	</name>
@x_old	指向旧版本消息中的需要比较的第一个结点 如<header>标签下的第一个子结点
@x_new	指向新版本消息中的需要比较的第一个结点
@type   比较的标签类型默认为header
*/
int IMIXMerge::singleCompare(rapidxml::xml_node<> *x_old, rapidxml::xml_node<> *x_new,TYPE type)
{
	rapidxml::xml_node<>* subOld = x_old->first_node();
	rapidxml::xml_node<>* subNew = x_new->first_node();
	rapidxml::xml_node<>* newTemp = subNew;
	
	//以旧消息里面的为准到新消息里面去寻找
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
		//到新的消息里面去遍历当前结点下的所有结点
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
						m_strLog<<"required属性不一致 name = "<<o_name<<"  旧版本的必输属性 required = "<<o_required<<"   旧版本的必输属性 required = "<<n_required<<std::endl;
						//把是否必须不匹配的以旧版本的为准
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
		//没找到结点
		if(NULL == newTemp)
		{
			//把缺少的节结合到新的版本的doc对里边
			rapidxml::xml_node<>* child = NULL;
			rapidxml::xml_attribute<> *attr = NULL;
			std::string req,tagName;
			//构建一个结点
			child = m_newDoc->allocate_node(rapidxml::node_element,subOld->name());
			attr = subOld->first_attribute();

			tagName = subOld->name();
			if(subOld->first_attribute(ATTR_REQUIR))
			{
				req = subOld->first_attribute(ATTR_REQUIR)->value();
			}
				
			appendAttr(child,attr);
			
			//若是component下面的结点且为必输则总是在最前边插入
			if((req == "Y" || req == "y") && type == COMPONENTS)
			{
				x_new->prepend_node(child);
			}//总是在最后边追加
			else
			{
				x_new->append_node(child);
			}
			
		}

		newTemp = subNew;//从头开始遍历
		subOld = subOld->next_sibling();
	}

	return 1;
}

/*
比较fields及下面的标签
@x_old	指向旧版本消息中的fields的第一个结点
@x_new	指向新版本消息中的fields的第一个结点
@type   比较的标签类型默认为fields
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
					//相同域但属性不同
					m_strLog<<"域的type属性不一致 name = "<<o_name<<"  旧版本的 type = "<<o_type<<"  新版本的 type = "<<n_type<<std::endl;

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
比较messages标签
@x_old	指向旧版本消息中的messages的第一个结点
@x_new	指向新版本消息中的messages的第一个结点
@type   比较的标签类型默认为messages
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
					m_strLog<<"消息标签中msgtype属性不一致 name = "<<o_name<<"  旧版本的类型为: "<<o_type<<"  旧版本的类型为: "<<n_type<<std::endl;
				}

				if(o_cat != n_cat)
				{
					m_strLog<<"消息标签中msgcat属性不一致 name = "<<o_name<<"  旧版本的类型为: "<<o_cat<<"  旧版本的类型为: "<<n_cat<<std::endl;
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
比较component标签
@x_old	指向旧版本消息中的components的第一个结点
@x_new	指向新版本消息中的components的第一个结点
@type   比较的标签类型默认为comonents
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
			//在发现compnent没找到之后把对应的compnent下面的结点追加进去
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
				//std::cout<<"components 下面的"<<sub2Old->first_attribute()->value()<<"子group没"<<std::endl;

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
追加一个component结点,其以下的结点一同追加
@des	需要追加到的地方	
@src	指向缺失的component
*/
int IMIXMerge::appendCmpnt(rapidxml::xml_node<> *des, rapidxml::xml_node<> *src)
{
	rapidxml::xml_node<> *a_cmpnt_nd = NULL;			
	rapidxml::xml_attribute<> *a_cmpnt_attr = NULL;

	//构建一个新的component结点
	a_cmpnt_nd = m_newDoc->allocate_node(rapidxml::node_element,src->name());		
	a_cmpnt_attr = src->first_attribute();

	//component的属性
	appendAttr(a_cmpnt_nd, a_cmpnt_attr);

	//获取要追加的源头的下一个结点
	rapidxml::xml_node<> * subSrc = src->first_node();
	while(subSrc != NULL)			
	{
		rapidxml::xml_node<> *a_cmpnt_chd_nd = NULL;  //component下面的新结点
		rapidxml::xml_attribute<> *attr = NULL;	  //新结点的属性
		std::string nodeName = subSrc->name();
		if (nodeName == "group")
		{
			//有group结点时把结点一个一个追加
			appendGroup(a_cmpnt_chd_nd, subSrc,COMPONENTS);
			a_cmpnt_nd->append_node(a_cmpnt_chd_nd);	//追加到最外面的component结点

		}else
		{
			//构建单个结点含field or component
			a_cmpnt_chd_nd = m_newDoc->allocate_node(rapidxml::node_element,subSrc->name());
			attr = subSrc->first_attribute();
			
			//追加结点的属性
			appendAttr(a_cmpnt_chd_nd,attr);

			//追加到最外面的component结点
			a_cmpnt_nd->append_node(a_cmpnt_chd_nd);	
		}
		subSrc = subSrc->next_sibling();
	}
	//最后把新建的component结点追加到目标位置
	des->append_node(a_cmpnt_nd);
	return 1;
}

/************************************************************************/
/* 新增结点类似
	<name attr=...>
		<subname attr=..>
		</subname>
	</name>
	
	@des	需要添加的结点
	@src	指向缺失的结点	
	@type	是结点的类型以便添加属性 
	*/
	
/************************************************************************/
int IMIXMerge::appendGroup(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src,TYPE type)
{
	rapidxml::xml_attribute<> *attr = NULL;

	//先建一个group父结点，之后再把子结点加进来
	des = m_newDoc->allocate_node(rapidxml::node_element,src->name());
	attr = src->first_attribute();
	
	appendAttr(des,attr);

	rapidxml::xml_node<> *subGroup = src->first_node();

	while (subGroup != NULL)
	{
		rapidxml::xml_node<> *subNode = NULL; //指向group标签下的子结点
		rapidxml::xml_attribute<> *subAttr = NULL;

		//构建结点
		subNode = m_newDoc->allocate_node(rapidxml::node_element,subGroup->name());
		subAttr = subGroup->first_attribute();

		appendAttr(subNode,subAttr);

		//再把结点放到创建好的group结点
		des->append_node(subNode);	

		subGroup = subGroup->next_sibling();
	}
	return 1;
}

/*
追加field下面的value结点
@des	需要追加到的地方	
@src	指向缺失value结点的field
*/
int IMIXMerge::appendFldValue(rapidxml::xml_node<> *&des, rapidxml::xml_node<> *src)
{
	while(src != NULL)
	{
		//构建结点
		rapidxml::xml_node<> *valueNode = m_newDoc->allocate_node(rapidxml::node_element,src->name());
		rapidxml::xml_attribute<> *attr = src->first_attribute();

		//追加属性
		appendAttr(valueNode,attr);

		//追加结点
		des->append_node(valueNode);

		src = src->next_sibling();
	}
	return 1;
}

/*
遍历每个结点并放到结点上
@des	需要添加属性的结点
@attr	指向结点上面的属性
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
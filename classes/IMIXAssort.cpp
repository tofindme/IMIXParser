#include "IMIXAssort.h"

IMIXDocument::IMIXDocument()
{
	extRoot = NULL;
	m_doc = new rapidxml::xml_document<>();
}

IMIXDocument::~IMIXDocument()
{
	SAFE_FREE(m_doc);
}

//�����������֣���׷�Ӹ�����
void IMIXDocument::initDeclar(rapidxml::xml_node<> *node)
{
	rapidxml::xml_node<> *element =  m_doc->allocate_node(rapidxml::node_declaration,node->name());

	//�����������Բ���ӵ������
	rapidxml::xml_attribute<> *attr = node->first_attribute();

	appendAttr(attr, element);

	//��node_declaration����ָ������xml�Ľ�㣬֮������������һ�ļ���
	m_doc->append_node(element);
}


//��ӽ�������ֻ��Ϊһ���ĵ����
void IMIXDocument::appendAttr(rapidxml::xml_attribute<> *&attr, rapidxml::xml_node<> *&node)
{
	for(; attr != NULL; attr = attr->next_attribute())
	{
		rapidxml::xml_attribute<> *att = m_doc->allocate_attribute(attr->name(),attr->value());
		node->append_attribute(att);
	}
}


void IMIXDocument::initEelements(rapidxml::xml_node<> *node)
{
	rapidxml::xml_node<> *element =  m_doc->allocate_node(rapidxml::node_element,node->name());
	rapidxml::xml_attribute<> *attr = node->first_attribute();

	//�����������Բ���ӵ������
	appendAttr(attr, element);

	//�ٴ���header/trailer/messages/components/fields���
	for(int i = 0 ; i < TAGS; i++)
	{
		rapidxml::xml_node<> *tag =  NULL;
		switch(i)
		{
		case 0:
			tag = m_doc->allocate_node(rapidxml::node_element,TAG_HEADER);
			break;
		case 1:
			tag = m_doc->allocate_node(rapidxml::node_element,TAG_TRAILER);
			break;
		case 2:
			tag = m_doc->allocate_node(rapidxml::node_element,TAG_MESSAGES);
			break;
		case 3:
			tag = m_doc->allocate_node(rapidxml::node_element,TAG_COMPONENTS);
			break;
		case 4:
			tag = m_doc->allocate_node(rapidxml::node_element,TAG_FIELDS);
			break;
		default:
			//...
			break;
		}
		element->append_node(tag);
	}

	//ָ���һ������㣬�������ӽ��ĸ����
	m_root = element;

	//��node_document����ָ������xml�Ľ�㣬֮������������һ�ļ���
	m_doc->append_node(element);
}



//headָ��doc��ߵ�header��ǩ
void IMIXDocument::appendHeader(rapidxml::xml_node<> *head)
{
	if(!head)
		return;
	rapidxml::xml_node<> *tag = head->first_node();
	rapidxml::xml_node<> *hdTag = m_root->first_node(TAG_HEADER);

	assert(hdTag != NULL);
	while(NULL != tag)
	{
		rapidxml::xml_node<> *element = m_doc->allocate_node(rapidxml::node_element,tag->name());
		rapidxml::xml_attribute<> *attr = tag->first_attribute();

		//�ѵ�ǰ�������Զ���ӵ������
		appendAttr(attr,element);
		hdTag->append_node(element);
		
		//��Ӧ����fields������Ӷ�Ӧ��field
		if(strcmp(tag->name(),TAG_FIELD) == 0)
		{
			//if(!existFld(tag->first_attribute("name")->value()))
			{
				appendField(tag->first_attribute(ATTR_NAME)->value());
			}
		}//��component
		else if(strcmp(tag->name(),TAG_COMPONENT) == 0)
		{
			//if(!existCmpnt(tag->first_attribute("name")->value()))
			{
				appendCmpnt(tag->first_attribute(ATTR_NAME)->value());
			}
		}
		tag = tag->next_sibling();
	}
}

//headָ��doc��ߵ�trailer��ǩ
void IMIXDocument::appendTrailer(rapidxml::xml_node<> *_trail)
{
	if(!_trail)
		return;
	rapidxml::xml_node<> *tag = _trail->first_node();
	rapidxml::xml_node<> *trail = m_root->first_node(TAG_TRAILER);

	assert(trail != NULL);
	while(NULL != tag)
	{
		rapidxml::xml_node<> *node = m_doc->allocate_node(rapidxml::node_element,tag->name());
		rapidxml::xml_attribute<> *attr = tag->first_attribute();

		//�ѵ�ǰ�������Զ���ӵ������
		appendAttr(attr,node);
		trail->append_node(node);

		//��Ӧ����fields������Ӷ�Ӧ��field
		if(strcmp(tag->name(),TAG_FIELD) == 0)
		{
			appendField(tag->first_attribute(ATTR_NAME)->value());
		}
		else if(strcmp(tag->name(),TAG_COMPONENT) == 0)
		{
			appendCmpnt(tag->first_attribute(ATTR_NAME)->value());
		}
		tag = tag->next_sibling();
	}
}

//׷��msgs���
void IMIXDocument::appendMsgs(rapidxml::xml_node<> *msgs)
{
	if(!msgs)
		return;
	rapidxml::xml_node<> *message = m_doc->allocate_node(rapidxml::node_element,msgs->name());
	rapidxml::xml_attribute<> *msgAttr = msgs->first_attribute();
	rapidxml::xml_node<> *messages = m_root->first_node(TAG_MESSAGES);
	rapidxml::xml_node<> *msg = msgs->first_node();

	appendAttr(msgAttr,message);
	while(msg != NULL)
	{
		rapidxml::xml_node<> *tag = m_doc->allocate_node(rapidxml::node_element,msg->name());
		rapidxml::xml_attribute<> *attr = msg->first_attribute();
		appendAttr(attr,tag);
		message->append_node(tag);
		if (strcmp(msg->name(), TAG_FIELD) == 0 )
		{
			if(!existFld(msg->first_attribute(ATTR_NAME)->value()))
			{
				appendField(msg->first_attribute(ATTR_NAME)->value());
			}
		}
		//�����component
		else if(strcmp(msg->name(), TAG_COMPONENT) == 0 )
		{
			if(!existCmpnt(msg->first_attribute(ATTR_NAME)->value()))
			{
				appendCmpnt(msg->first_attribute(ATTR_NAME)->value());
			}
		}	
		msg = msg->next_sibling();
	}
	messages->append_node(message);
}


/*
����name���Ե�ֵ����field,����Ӷ�Ӧ�Ľ��
@name	����ӵ�field����
*/
void IMIXDocument::appendField(const std::string name)
{
	rapidxml::xml_node<> *fld = findField(name);
	rapidxml::xml_node<> *tag = m_doc->allocate_node(rapidxml::node_element,fld->name());
	rapidxml::xml_node<> *fields = m_root->first_node(TAG_FIELDS);	//ָ��m_appDoc��fields��ǩ

	if(fld->first_node() != NULL)
	{
		appendFldValue(fld,tag);
		fields->append_node(tag);
	}
	else
	{
		rapidxml::xml_attribute<> *attr = fld->first_attribute();
		appendAttr(attr,tag);
		fields->append_node(tag);
	}
}

/*
׷��field�����value���
@fld	value�ĸ����
@node	��ӵ�node�ڵ��ϵ�ָ��
*/
void IMIXDocument::appendFldValue(rapidxml::xml_node<> *fld,rapidxml::xml_node<> *node)
{
	rapidxml::xml_attribute<> *fldAttr = fld->first_attribute();
	rapidxml::xml_node<> *subNode = fld->first_node();

	appendAttr(fldAttr,node);
	while(subNode != NULL)
	{
		rapidxml::xml_node<>* tag = m_doc->allocate_node(rapidxml::node_element,subNode->name());
		rapidxml::xml_attribute<> *attr = subNode->first_attribute();

		appendAttr(attr,tag);
		node->append_node(tag);	
		subNode = subNode->next_sibling();
	}
}

//ָ���ⲿdoc������һ�㸸���
void IMIXDocument::setExtRoot(rapidxml::xml_node<> *rt)
{
	extRoot = rt;
}

/*
׷��component�µĽ��
@name	��Ҫ��ӵ�component��������
*/
void IMIXDocument::appendCmpnt(const std::string name)
{
	rapidxml::xml_node<> *cmpnt = findCmpnt(name);
	rapidxml::xml_node<> *cmpnts = m_root->first_node(TAG_COMPONENTS);
	rapidxml::xml_node<> *element = m_doc->allocate_node(rapidxml::node_element,cmpnt->name());
	rapidxml::xml_attribute<> *cmpntAttr = cmpnt->first_attribute();

	cmpnt = cmpnt->first_node();
	appendAttr(cmpntAttr,element);
	while(cmpnt != NULL)
	{
		rapidxml::xml_node<> *tag = m_doc->allocate_node(rapidxml::node_element,cmpnt->name());
		rapidxml::xml_attribute<> *attr = cmpnt->first_attribute();

		appendAttr(attr,tag);

		if(strcmp(cmpnt->name(),TAG_GROUP) == 0 )
		{
			//if(!existFld(cmpnt->first_attribute(ATTR_NAME)->value()))
			{
				if(!existFld(cmpnt->first_attribute(ATTR_NAME)->value()))
				{
					appendField(cmpnt->first_attribute(ATTR_NAME)->value());
				}
				appendGroup(cmpnt,tag);
			}
		}
		else if(strcmp(cmpnt->name(), TAG_FIELD) == 0 )
		{
			if(!existFld(cmpnt->first_attribute(ATTR_NAME)->value()))
			{
				appendField(cmpnt->first_attribute(ATTR_NAME)->value());
			}
		}
		else if(strcmp(cmpnt->name(), TAG_COMPONENT) == 0 )
		{
			if(!existCmpnt(cmpnt->first_attribute(ATTR_NAME)->value()))
			{
				appendCmpnt(cmpnt->first_attribute(ATTR_NAME)->value());
			}
		}
		element->append_node(tag);
		cmpnt = cmpnt->next_sibling();
	}
	cmpnts->prepend_node(element);
}

/*
����component��λ��
@name	��Ҫ���ҵ�component����
*/
rapidxml::xml_node<> * IMIXDocument::findCmpnt(const std::string name)
{
	rapidxml::xml_node<> * cmpnts = extRoot->first_node(TAG_COMPONENTS);
	rapidxml::xml_node<> * cmpnt = cmpnts->first_node();

	while(cmpnt != NULL)
	{
		std::string na;
		na = cmpnt->first_attribute(ATTR_NAME)->value();
		if(na == name)
			return cmpnt;
		cmpnt = cmpnt->next_sibling();
	}
	return NULL;

}

/*
׷��group�µ������ӽ��
@cmpnt	ָ����Ҫ��ӵ�group���
@grp	gropu�ڵ�
*/

void IMIXDocument::appendGroup(rapidxml::xml_node<> *cmpnt, rapidxml::xml_node<> *&grp)
{
	rapidxml::xml_node<> *fields = cmpnt->first_node();

	while(fields)
	{
		rapidxml::xml_node<> *tag = m_doc->allocate_node(rapidxml::node_element,fields->name());
		rapidxml::xml_attribute<> *attr = fields->first_attribute();
		
		appendAttr(attr,tag);
		if(strcmp(fields->name(), TAG_FIELD) == 0)
		{
			if(!existFld(fields->first_attribute(ATTR_NAME)->value()))
			{
				appendField(fields->first_attribute(ATTR_NAME)->value());
			}
		}
		else if(strcmp(fields->name(), TAG_COMPONENT) == 0)
		{
			if(!existCmpnt(fields->first_attribute(ATTR_NAME)->value()))
			{
				appendCmpnt(fields->first_attribute(ATTR_NAME)->value());
			}
		}

		grp->append_node(tag);

		fields = fields->next_sibling();
	}
}


//�ҵ�field�Ľ��
rapidxml::xml_node<> * IMIXDocument::findField(const std::string name)
{
	rapidxml::xml_node<> * flds = extRoot->first_node(TAG_FIELDS);
	rapidxml::xml_node<> * fld = flds->first_node();

	while(fld != NULL)
	{
		std::string na;
		na = fld->first_attribute(ATTR_NAME)->value();
		if(na == name)
			return fld;
		fld = fld->next_sibling();
	}
	return NULL;
}

void IMIXDocument::outFile(const char *fileName)
{
	std::ofstream outXML;
	outXML.open(fileName,std::ios_base::out | std::ios_base::trunc);
	outXML<<*m_doc;
}

//�ж�fld�Ƿ��Ѿ����ڣ���������Ҫ�ٴβ��뵽doc
bool IMIXDocument::existFld(std::string name)
{
	rapidxml::xml_node<> * flds = m_root->first_node(TAG_FIELDS);
	rapidxml::xml_node<> * fld = flds->first_node();

	while(fld != NULL)
	{
		std::string na;
		na = fld->first_attribute(ATTR_NAME)->value();
		if(na == name)
			return true;
		fld = fld->next_sibling();
	}
	return false;
}

//�ж�compnent�Ƿ��Ѿ����ڣ���������Ҫ�ٴβ��뵽doc
bool IMIXDocument::existCmpnt(std::string name)
{
	rapidxml::xml_node<> * cmpnts = m_root->first_node(TAG_COMPONENTS);
	rapidxml::xml_node<> * cmpnt = cmpnts->first_node();

	while(cmpnt != NULL)
	{
		std::string na;
		na = cmpnt->first_attribute(ATTR_NAME)->value();
		if(na == name)
			return true;
		cmpnt = cmpnt->next_sibling();
	}
	return false;

}



IMIXAssort::IMIXAssort(void)
{
	m_file = NULL;
	m_doc = NULL;
	m_root = NULL;
	m_app = new IMIXDocument();
	m_admin = new IMIXDocument();
}


IMIXAssort::~IMIXAssort(void)
{
	SAFE_FREE(m_file);
	SAFE_FREE(m_doc);

	SAFE_FREE(m_app);
	SAFE_FREE(m_admin);
	
	m_root = NULL;
}


void IMIXAssort::loadFile(const char *fileName)
{
	SAFE_FREE(m_file);
	SAFE_FREE(m_doc);

	m_file = new rapidxml::file<>(fileName);
	m_doc = new rapidxml::xml_document<>();

	m_doc->parse<0x20>(m_file->data());	//0x20�������xml�������ּ�<?xml...?>	

	m_root = m_doc->first_node();
	
	//��ʼ����doc
	iteratorDoc();
}

//������Ϣ
void IMIXAssort::iteratorDoc(void)
{
	for(;m_root != NULL; m_root = m_root->next_sibling())
	{
		if(rapidxml::node_declaration == m_root->type())
		{
			//��doc��ߴ���һ��declaration
			m_app->initDeclar(m_root);
			m_admin->initDeclar(m_root);
		}
		else if(rapidxml::node_element == m_root->type())
		{
			m_app->setExtRoot(m_root);
			//��ʼ��xml����Ϣ�ṹ  header/trailer����
			appConstru();

			m_admin->setExtRoot(m_root);
			//��ʼ��xml����Ϣ�ṹ  header/trailer����
			adminConstru();

			//����messages��� ��ʼapp��admin��Ϣ����
			iteratorMsgs(m_root->first_node(TAG_MESSAGES));


// 			m_app->outFile("app.xml");
// 			m_admin->outFile("admin.xml");
		}
	}
}

//������Ϣ�е�messages�����н��
void IMIXAssort::iteratorMsgs(rapidxml::xml_node<> *msgs)
{
	rapidxml::xml_node<> *msg = msgs->first_node();
	while(msg != NULL)
	{
		std::string msgcat = msg->first_attribute("msgcat")->value();
		if(msgcat == "app")
		{//��app��Ϣ�ŵ��ڴ�
			m_app->appendMsgs(msg);
		}
		else if(msgcat == "admin")
		{
			m_admin->appendMsgs(msg);
		}
		else
		{
			//...
		}
		msg = msg->next_sibling();
	}
}


//��doc�ﴴ��һ��node_element�Ĳ㼶�ṹֻ�нṹ
//�� header/trailer/messages/components/fields��ǩ
void IMIXAssort::appConstru(void)
{
	m_app->initEelements(m_root);
	m_app->appendHeader(m_root->first_node(TAG_HEADER));	
	m_app->appendTrailer(m_root->first_node(TAG_TRAILER));
}


void IMIXAssort::adminConstru(void)
{
	m_admin->initEelements(m_root);
	m_admin->appendHeader(m_root->first_node(TAG_HEADER));	
	m_admin->appendTrailer(m_root->first_node(TAG_TRAILER));
}

void IMIXAssort::appToFile(const char *name)
{
	if(name == NULL)
		m_app->outFile("app.xml");
	else
		m_app->outFile(name);
}


void IMIXAssort::adminToFile(const char *name)
{
	if(name == NULL)
		m_admin->outFile("admin.xml");
	else
		m_admin->outFile(name);
}

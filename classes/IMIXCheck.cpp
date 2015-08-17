#include "IMIXCheck.h"


IMIXCheck::IMIXCheck(void):extraFld(40000)
{
	m_file = NULL;
	m_doc = NULL;
}

IMIXCheck::~IMIXCheck(void)
{
	SAFE_FREE(m_file);
	SAFE_FREE(m_doc);

	Group::iterator ite = m_mGroup.begin();
	while(ite != m_mGroup.end())
	{
		delete ite->second.second;
		ite++;
	}


}

/*
判断是否是一个重复组
@msg	消息的类型
@field	域号
*/
bool IMIXCheck::isGroupFld(std::string msg, int field)
{
	if(m_mGroup.find(std::make_pair(msg,field)) != m_mGroup.end())
	{
		return true;
	}else
	{
		Group::iterator ite = m_mGroup.begin();
		while(ite != m_mGroup.end())
		{
			std::pair<std::string, int> parFirst = ite->first;
			std::pair<int, IMIXCheck*> par = ite->second;
			IMIXCheck *dd = par.second;

			if(parFirst.first != msg)
			{
				ite++;
				continue;
			}
			if(dd->isGroupFld(msg,field))
			{
				return true;
			}
			ite++;
		}
	}
	return false;
}

/*
判断是否需要检测的域
@field	域号
*/
bool IMIXCheck::shouldCheck(int field)
{
	return field < getExtraFld();
}

/*
检测消息部分
@begin	指向消息体的首地址
@msglen	记录消息体的长度
*/
int IMIXCheck::parseMsg(const char *begin, int &msgLen)
{
	char *temp = (char*)begin;

	map<int,std::string> msgMap;

	while(temp)
	{
		int field = 0;
		std::string value;
		field = atoi(temp);

		//是消息尾的部分的时候就退出
		if(!findFldInMsg(field) && !isHeaderFld(field) && isTrailerFld(field))
			break;

		if(isHeaderFld(field) || isTrailerFld(field))
		{
			return IMIX_POSI_ERR;
			break;
		}

		temp = getValue2Next(temp,value);

		//判断是否需要检测的域
		if(!shouldCheck(field))
			continue;

		int skip = 0;
		if(isGroupFld(msgType,field))
		{
			int ret = parseGroup(field,value,temp,skip);
			if(ret != IMIX_SUCCESS)
				return ret;
			temp += skip;
		}
		else
		{//不是group的时候判断域号存在与否和value的合法性

			if(!findFldInMsg(field) || !lookupFld(field) )
			{
				return IMIX_NOT_FIND_ERR;
			}
			if(hasValue(field) && !findValueInFld(field,value))
			{
				return IMIX_VALUE_ERR;
			}

			if(msgMap.insert(std::make_pair(field,value)).second == false)
				return IMIX_REPEAT_ERR;//有重复的
		}
	}
	msgLen = temp - begin;	

	FldIsRequired::iterator ite = m_mMsgFldsIsBool.begin();
	while(ite != m_mMsgFldsIsBool.end())
	{
		if(ite->second && msgMap.find(ite->first) == msgMap.end())
		{
			return IMIX_REQ_ERR;
		}
		ite++;
	}

	return IMIX_SUCCESS;
}


bool IMIXCheck::isTrailerFld(int num)
{
	return m_mTrailFlds.find(num) != m_mTrailFlds.end();
}


bool IMIXCheck::parseLastFld(const char *buf)
{
	char *s = strrchr((char*)buf,'=');
	char *t = s;
	t++;//指向等号后边的字符

	while(*s != '\001')
	{
		s--;
	}
	s++;

	int num = atoi(s);

	if(num != 10)
		return false;

	char *last = strrchr(s,'\001');

	std::string va;
	va.append(t,last-t);

	//if(!findValueInFld(num,va))
	//	return false;

	return true;

}

/*
解析消息尾
@begin	指向消息尾部分的首地址
@trailerLen	记录消息尾的长度
*/
int IMIXCheck::parseTrailer(const char* begin, int &trailerLen)
{
	if(!parseLastFld(begin))
		return IMIX_TRAILER_ERR;

	char * temp = (char*)begin;
	char * end = temp + strlen(temp);
	map<int,std::string> trailMap;

	while(temp != end)
	{
		int num = 0;
		std::string va;
		num = atoi(temp);
		temp = getValue2Next(temp,va);

		if(isHeaderFld(num) || findFldInMsg(num))
		{
			return IMIX_POSI_ERR;
			break;
		}

		if(!shouldCheck(num))
			continue;

		if(!isTrailerFld(num) || !lookupFld(num))
			return IMIX_NOT_FIND_ERR;

		if(hasValue(num) && !findValueInFld(num,va))
			return IMIX_VALUE_ERR;//return false;

		if(trailMap.insert(std::make_pair(num,va)).second == false)
			return IMIX_REPEAT_ERR;//
	}
	trailerLen = end - begin;

	//查找必须域
	FldIsRequired::iterator ite = m_mTrailFlds.begin();
	while(ite != m_mTrailFlds.end())
	{
		if(ite->second && trailMap.find(ite->first) == trailMap.end())
		{
			return IMIX_REQ_ERR;
		}
		ite++;
	}
	return IMIX_SUCCESS;
}

bool IMIXCheck::headerIsOrder(const char *buf)
{
	char *temp = (char*)buf;
	int count = 1;
	while(count < 4)
	{
		int num = 0;
		std::string va;
		num = atoi(temp);
		temp = getValue2Next(temp,va);
		if(temp == NULL)
			return false;

		if(count == 1 && num == 8)
		{
			//do nothing
		}
		else if(count == 2 && num == 9)
		{
			//do nothing
		}
		else if(count == 3 && num == 35)
		{
			msgType = va;	//保存消息类型
		}else
		{
			return false;
		}
		count++;
	}
	return true;

}

bool IMIXCheck::isHeaderFld(int field)
{
	return m_mHeadFlds.find(field) != m_mHeadFlds.end();
}

bool IMIXCheck::hasValue(int field)
{
	return m_mValueToFld[field].size() > 0?true:false;
}

/*
解析消息头
@buf			指向消息头的位置
@headerLen		记录消息头找长度
*/
int IMIXCheck::parseHeader(const char* buf, int &headerLen)
{
	if(!headerIsOrder(buf))
		return IMIX_HEADER_ERR;

	map<int,std::string> headMap;
	char *temp = (char*)buf;

	while(temp)
	{
		int field = 0;
		std::string va;
		field = atoi(temp);

		//当前域不属于消息头的域且它下一个域属于消息头时说明位置已经错乱
		if(!isHeaderFld(field) && isHeaderFld(atoi(getValue2Next(temp,va))) && shouldCheck(field))		
		{
			return IMIX_POSI_ERR;
			break;
		}

		if(!isHeaderFld(field) && (findFldInMsg(field) || isTrailerFld(field)))		
		{//消息头的域检测完毕，中断继续后边的检测
			break;
		}

		temp = getValue2Next(temp,va);

		//对可扩展域不进行检测
		if(!shouldCheck(field))
			continue;

		if(isGroupFld("header",field))
		{
			int skip = 0;
			int ret = parseHeaderGroup(field,va,temp,skip);
			if(ret != IMIX_SUCCESS)
				return ret;
			temp += skip;
		}

		if(!isHeaderFld(field) || !lookupFld(field))
			return IMIX_NOT_FIND_ERR;

		if(hasValue(field) && !findValueInFld(field,va))
			return IMIX_VALUE_ERR;

		if(headMap.insert(std::make_pair(field,va)).second == false)
			return IMIX_REPEAT_ERR;
	}
	headerLen = temp - buf;

	//查找必须域是否都有
	FldIsRequired::iterator ite = m_mHeadFlds.begin();
	while(ite != m_mHeadFlds.end())
	{
		if(ite->second && (headMap.find(ite->first) == headMap.end()))
		{
			return IMIX_REQ_ERR;
		}
		ite++;
	}

	return IMIX_SUCCESS;
}


char * IMIXCheck::getValue2Next(const char *str, std::string &value)
{
	char *temp = (char*)str;
	value = "";

	char *pkey = NULL;
	pkey = strstr(temp,"=");
	if(pkey == NULL)
		return NULL;
	pkey++;

	char *va = NULL;
	va = strstr(pkey,"\001");
	if(va != NULL)
	{
		value.append(pkey,va-pkey);
		va++;
		return va;
	}
	return NULL;
}

bool IMIXCheck::fldInGroup(std::string msg, int num)
{
	return m_mMsgFlds[msg].find(num) != m_mMsgFlds[msg].end();
}

int IMIXCheck::parseHeaderGroup(int field,std::string value, const char *buf,int &skip)
{
	char *temp = (char*)buf;
	int count = atoi(value.c_str());

	if(m_mGroup.find(std::make_pair("header",field)) != m_mGroup.end())
	{
		std::pair<int,IMIXCheck*> pr = m_mGroup[std::make_pair("header",field)];

		IMIXCheck *groupDD = pr.second;
		groupDD->msgType = "header";
		groupDD->m_mValueToFld = this->m_mValueToFld;
		groupDD->m_sFlds = this->m_sFlds;
		groupDD->m_mFldKeyNo = this->m_mFldKeyNo;
		map<int,std::string> groupMap;
		bool groupBreak = false;
		int rCount = 1;;
		int fldCount = 1;
		while(count && temp != NULL)
		{
			int num = 0;
			std::string val;

			if(temp != NULL)
				num = atoi(temp);

			if(num == pr.first)
				;//count--;
			else
				return IMIX_GRP_FIRST_ERR;

			if(temp != NULL)
				temp = getValue2Next(temp,val);

			//相应的域值
			if(groupDD->hasValue(num) && !groupDD->findValueInFld(num,val))
			{
				return IMIX_VALUE_ERR;
			}
			groupMap.insert(std::make_pair(num,val));


			for(int i = 0; i < groupDD->m_mMsgFlds["header"].size() - 1; i++)
			{
				int grpFld = atoi(temp);
				if(grpFld == pr.first)
				{
					rCount++;
					break;
				}
				
				fldCount++;

				if(groupDD->isGroupFld("header",grpFld))
				{
					int groupLen = 0;
					temp = getValue2Next(temp,val);
					int ret = groupDD->parseHeaderGroup(grpFld,val,temp,groupLen);
					if(ret != IMIX_SUCCESS)
						return ret;
					temp += groupLen;
				}
				else
				{
					if(!groupDD->fldInGroup("header",grpFld) /*&&
						(groupDD->isGroupFld("header",atoi(getValue2Next(temp,val))) ||
						!groupDD->fldInGroup("header",atoi(getValue2Next(temp,val))))*/)
					{
						if(groupDD->fldInGroup("header",atoi(getValue2Next(temp,val))) &&
							!groupDD->isGroupFld("header",atoi(getValue2Next(temp,val))))
							return IMIX_NOT_FIND_ERR;
						else
						{
							groupBreak = true;
							break;
						}
					}
					temp = getValue2Next(temp,val);
					
					//到相应的group下面查找存放的field
					if((!groupDD->fldInGroup("header",grpFld) || !groupDD->lookupFld(grpFld)))
					{
						return IMIX_NOT_FIND_ERR;
					}

					//相应的域值
					if(groupDD->hasValue(grpFld) && !groupDD->findValueInFld(grpFld,val))
					{
						return IMIX_VALUE_ERR;
					}
					if(groupMap.insert(std::make_pair(grpFld,val)).second == false)
						return IMIX_GRP_REPEAT_ERR;
				}
			}

			//必须域检测
			FldIsRequired::iterator ite = groupDD->m_mMsgFldsIsBool.begin();
			while(ite != groupDD->m_mMsgFldsIsBool.end())
			{
				if(ite->second && groupMap.find(ite->first) == groupMap.end())
				{
					return IMIX_REQ_ERR;
				}
				ite++;
			}			
			groupMap.clear();

			if(groupDD->m_mMsgFlds["header"].size() == fldCount && rCount == count)
				break;

			if(groupBreak)
			{
				if(count == rCount)
					break;
				else
					return IMIX_GRP_COUNT_ERR;
			}
		}	
		skip = temp - buf;

		return IMIX_SUCCESS;
	}
	else
	{
		Group::iterator ite = m_mGroup.begin();
		while(ite != m_mGroup.end())
		{
			std::pair<std::string, int> prfirst = ite->first;
			std::pair<int, IMIXCheck*> pr = ite->second;
			IMIXCheck *dd = pr.second;

			if(prfirst.first != "header")
			{
				ite++;
				continue;
			}

			dd->msgType = "header";
			dd->m_mValueToFld = this->m_mValueToFld;
			dd->m_sFlds = this->m_sFlds;
			dd->m_mFldKeyNo = this->m_mFldKeyNo;
			int ret = dd->parseHeaderGroup(field,value,buf,skip);
			if(ret != IMIX_SUCCESS)
				return ret;
			ite++;
		}
	}
	return IMIX_SUCCESS;
}

int IMIXCheck::parseGroup(int field,std::string value, const char *buf,int &skip)
{
	char *temp = (char*)buf;
	int count = atoi(value.c_str());

	if(m_mGroup.find(std::make_pair(msgType,field)) != m_mGroup.end())
	{
		std::pair<int,IMIXCheck*> pr = m_mGroup[std::make_pair(msgType,field)];

		IMIXCheck *groupDD = pr.second;
		groupDD->msgType = this->msgType;
		groupDD->m_mValueToFld = this->m_mValueToFld;
		groupDD->m_sFlds = this->m_sFlds;
		groupDD->m_mFldKeyNo = this->m_mFldKeyNo;
		map<int,std::string> groupMap;
		bool groupBreak = false;
		int fldCount = 0;
		int rCount = 1;;
		while(count && temp != NULL)
		{
			int num = 0;
			std::string val;

			if(temp != NULL)
				num = atoi(temp);

			if(num == pr.first)
				;//count--;
			else
				return IMIX_GRP_FIRST_ERR;
 
			if(temp != NULL)
				temp = getValue2Next(temp,val);

			//相应的域值
			if(groupDD->hasValue(num) && !groupDD->findValueInFld(num,val))
			{
				return IMIX_VALUE_ERR;
			}
			groupMap.insert(std::make_pair(num,val));
			
		
			for(int i = 0; i < groupDD->m_mMsgFlds[msgType].size() - 1; i++)
			{
				int grpFld = atoi(temp);
				if(grpFld == pr.first)
				{
					rCount++;
					break;
				}
				fldCount++;
				if(groupDD->isGroupFld(msgType,grpFld))
				{
					int groupLen = 0;
					temp = getValue2Next(temp,val);
					int ret = groupDD->parseGroup(grpFld,val,temp,groupLen);
					if(ret != IMIX_SUCCESS)
						return ret;
					temp += groupLen;
				}
				else
				{
					if(!groupDD->fldInGroup(msgType,grpFld) /*&&
						(groupDD->isGroupFld(msgType,atoi(getValue2Next(temp,val))) ||
						!groupDD->fldInGroup(msgType,atoi(getValue2Next(temp,val))))*/)
					{
						if(groupDD->fldInGroup(msgType,atoi(getValue2Next(temp,val))) &&
							!groupDD->isGroupFld(msgType,atoi(getValue2Next(temp,val))))
							return IMIX_NOT_FIND_ERR;
						else
						{
							groupBreak = true;
							break;
						}
					}
					temp = getValue2Next(temp,val);
					if(isHeaderFld(grpFld) || isTrailerFld(grpFld))
					{
						return IMIX_POSI_ERR;
					}
					//到相应的group下面查找存放的field
					if((!groupDD->fldInGroup(msgType,grpFld) || !groupDD->lookupFld(grpFld)))
					{
						return IMIX_NOT_FIND_ERR;
					}

					//相应的域值
					if(groupDD->hasValue(grpFld) && !groupDD->findValueInFld(grpFld,val))
					{
						return IMIX_VALUE_ERR;
					}
					if(groupMap.insert(std::make_pair(grpFld,val)).second == false)
						return IMIX_GRP_REPEAT_ERR;
				}
			}

			//必须域检测
			FldIsRequired::iterator ite = groupDD->m_mMsgFldsIsBool.begin();
			while(ite != groupDD->m_mMsgFldsIsBool.end())
			{
				if(ite->second && groupMap.find(ite->first) == groupMap.end())
				{
					return IMIX_REQ_ERR;
				}
				ite++;
			}			
			groupMap.clear();


			if(groupDD->m_mMsgFlds[msgType].size() == fldCount && rCount == count)
				break;

			if(groupBreak)
			{
				if(count == rCount)
					break;
				else
					return IMIX_GRP_COUNT_ERR;
			}
		}	
		skip = temp - buf;

		return IMIX_SUCCESS;
	}
	else
	{
		Group::iterator ite = m_mGroup.begin();
		while(ite != m_mGroup.end())
		{
			std::pair<std::string, int> prfirst = ite->first;
			std::pair<int, IMIXCheck*> pr = ite->second;
			IMIXCheck *dd = pr.second;

			if(prfirst.first != this->msgType)
			{
				ite++;
				continue;
			}

			dd->msgType = this->msgType;
			dd->m_mValueToFld = this->m_mValueToFld;
			dd->m_sFlds = this->m_sFlds;
			dd->m_mFldKeyNo = this->m_mFldKeyNo;
			int ret = dd->parseGroup(field,value,buf,skip);
			if(ret != IMIX_SUCCESS)
				return ret;
			ite++;
		}
	}
	return IMIX_SUCCESS;
}

bool IMIXCheck::findFldInMsg(int field)
{	
	if(m_mMsgFlds[msgType].find(field) != m_mMsgFlds[msgType].end())
	{
		return true;
	}else
	{
		Group::iterator ite = m_mGroup.begin();
		while(ite != m_mGroup.end())
		{
			std::pair<int, IMIXCheck*> par = ite->second;
			IMIXCheck *dd = par.second;
			dd->msgType = this->msgType;
			if(dd->findFldInMsg(field))
			{
				return true;
			}
			ite++;
		}
	}
	return false;
}

bool IMIXCheck::lookupFld(int field)
{
	return m_mFldKeyNo.find(field) != m_mFldKeyNo.end();
}

bool IMIXCheck::findValueInFld(int field,std::string value)
{
	return m_mValueToFld[field].find(value) != m_mValueToFld[field].end();
}

int IMIXCheck::validate(const char *buf, int len)
{
	char * begin = (char*)buf;
	char * end = (char*)buf + len;
	int headlen = 0;
	int msgLen = 0;
	int trailerLen = 0;

	int ret = 0;
	ret = parseHeader(buf,headlen);
	if(ret != IMIX_SUCCESS)
		return ret;

	begin += headlen;
	ret = parseMsg(begin,msgLen);
	if(ret != IMIX_SUCCESS)
		return ret;

	begin += msgLen;
	ret = parseTrailer(begin,trailerLen);
	if(ret != IMIX_SUCCESS)
		return ret;

	begin += trailerLen;

	if(begin != end)
		return IMIX_DATA_LOSE;

	return IMIX_SUCCESS;
}

void IMIXCheck::setXMLData(char *fileName)
{
	assert(fileName != NULL);

	if(m_file != NULL)
	{
		delete m_file;
		m_file = NULL;
	}

	if(m_doc != NULL)
	{
		delete m_doc;
		m_doc = NULL;
	}


	m_file = new rapidxml::file<>(fileName);
	m_doc = new rapidxml::xml_document<>();

	try
	{
		this->m_doc->parse<0>(m_file->data());
		this->m_root = this->m_doc->first_node();
	}
	catch (rapidxml::parse_error  e)
	{
		char *str = (char*)malloc(strlen(e.what())+601);
		memset(str,0,strlen(str));
		strcat(str,e.what());
		strcat(str," at ");
		strncat(str,e.where<char>(),600);
		delete m_doc;m_doc = NULL;
		//AfxMessageBox(str,MB_OK | MB_ICONERROR);
	}
	catch (std::exception e)
	{

	}
	readData();
}

void IMIXCheck::readData(void)
{
	//Fields
	rapidxml::xml_node<> *flds = m_root->first_node("fields");
	rapidxml::xml_node<> *fldNode = flds->first_node();

	while(fldNode)
	{
		std::string name,number;
		int num;

		name = fldNode->first_attribute("name")->value();
		number = fldNode->first_attribute("number")->value();
		num = atoi(number.c_str());

		addFlds(num);
		addFldName(num,name);

		rapidxml::xml_node<> *fldValueNode = fldNode->first_node();
		while(fldValueNode)
		{
			std::string valueEnum;
			valueEnum = fldValueNode->first_attribute("enum")->value();
			addFldValue(num,valueEnum);

			fldValueNode = fldValueNode->next_sibling();
		}
		fldNode = fldNode->next_sibling();
	}
#if 1
	//header
	rapidxml::xml_node<> *head = m_root->first_node("header");
	rapidxml::xml_node<> *headerNode = head->first_node();
	while(headerNode)
	{
		std::string tagName,attrName,attrIsReq;

		tagName = headerNode->name();
		attrName = headerNode->first_attribute("name")->value();
		attrIsReq = headerNode->first_attribute("required")->value();

		if(tagName == "field")
		{
			int num = lookupXMLFldNumber(attrName);
			addHeaderFlds(num,attrIsReq == "Y");
		}
		else if(tagName == "component")
		{
			addXMLComponent(attrName,attrIsReq == "Y","header",*this);
		}
		else if(tagName == "group")
		{
			addXMLGroup(headerNode,"header",*this);
		}

		headerNode = headerNode->next_sibling();
	}
#endif
	//msg
	rapidxml::xml_node<> *msgs = m_root->first_node("messages");
	rapidxml::xml_node<> *msgNode = msgs->first_node();

	while(msgNode)
	{
		std::string msgType,name;
		msgType = msgNode->first_attribute("msgtype")->value();
		name = msgNode->first_attribute("name")->value();

		addValueName(35,msgType,name);

		rapidxml::xml_node<> *msgFldNode = msgNode->first_node();
		while(msgFldNode)
		{
			std::string tagName,attName,attIsReq;

			tagName = msgFldNode->name();
			attName = msgFldNode->first_attribute("name")->value();
			attIsReq = msgFldNode->first_attribute("required")->value();
			if(tagName == "field")
			{
				int num = lookupXMLFldNumber(attName);
				addMsgFlds(msgType,num, attIsReq == "Y");
			}
			else if(tagName == "component")
			{
				addXMLComponent(attName,attIsReq == "Y",msgType,*this);
			}
			else if(tagName == "group")
			{
				int field = lookupXMLFldNumber(attName);
				addMsgFlds(msgType,field,attIsReq == "Y");
				addXMLGroup(msgFldNode, msgType, *this);
			}
			msgFldNode = msgFldNode->next_sibling();
		}
		msgNode = msgNode->next_sibling();
	}
#if 1
	//trailer
	rapidxml::xml_node<> *trail = m_root->first_node("trailer");
	rapidxml::xml_node<> *trailNode = trail->first_node();
	while(trailNode)
	{
		std::string tagName,attrName,attrIsReq;

		tagName = trailNode->name();
		attrName = trailNode->first_attribute("name")->value();
		attrIsReq = trailNode->first_attribute("required")->value();

		if(tagName == "field")
		{
			int num = lookupXMLFldNumber(attrName);
			addTrailerFlds(num,attrIsReq == "Y");
		}
		else if(tagName == "component")
		{
			addXMLComponent(attrName,attrIsReq == "Y","trailer",*this);
		}
		else if(tagName == "group")
		{
			addXMLGroup(trailNode,"trailer",*this);
		}

		trailNode = trailNode->next_sibling();
	}

#endif
}

int IMIXCheck::addXMLComponent(const std::string &name, bool isRequired, std::string msgType,IMIXCheck &dd)
{
	rapidxml::xml_node<> *cmpnts = m_root->first_node("components");
	rapidxml::xml_node<> *cmpntNode = cmpnts->first_node();
	int firstFld = 0;
	while(cmpntNode)
	{
		std::string attName = cmpntNode->first_attribute("name")->value();
		if(attName == name)
			break;
		cmpntNode = cmpntNode->next_sibling();
	}
	if(cmpnts)
	{
		rapidxml::xml_node<> *cmpntFldNode = cmpntNode->first_node();
		std::string tagName,attName,attIsReq;
		while(cmpntFldNode)
		{
			tagName = cmpntFldNode->name();
			attName = cmpntFldNode->first_attribute("name")->value();
			attIsReq = cmpntFldNode->first_attribute("required")->value();

			if(tagName == "field")
			{
				int num = lookupXMLFldNumber(attName);
				dd.addMsgFlds(msgType, num, (attIsReq == "Y" && isRequired) );

				if(firstFld == 0)
					firstFld = num;
			}else if(tagName == "component")
			{
				addXMLComponent(attName,attIsReq == "Y",msgType,dd);

			}
			else if(tagName == "group")
			{
				int field = lookupXMLFldNumber(attName);
				dd.addMsgFlds(msgType,field,attIsReq == "Y");
				addXMLGroup(cmpntFldNode, msgType, dd);
			}
			cmpntFldNode = cmpntFldNode->next_sibling();
		}
	}
	return firstFld;
}


int IMIXCheck::addXMLGroup(rapidxml::xml_node<> *groupNode, std::string msgType, IMIXCheck &dd)
{
	int group;
	group = lookupXMLFldNumber(groupNode);

	IMIXCheck *groupDD = new IMIXCheck;
	int field = 0;
	int first = 0;

	rapidxml::xml_node<> *fldNode = groupNode->first_node()	;

	while(fldNode)
	{
		std::string tagName,attName,attIsReq;
		tagName = fldNode->name();
		attName = fldNode->first_attribute("name")->value();
		attIsReq = fldNode->first_attribute("required")->value();

		if(tagName == "field")
		{
			field = lookupXMLFldNumber(attName);
			groupDD->addMsgFlds(msgType, field, attIsReq == "Y");

		}
		else if(tagName == "component")
		{
			field = addXMLComponent(attName, attIsReq == "Y", msgType, *groupDD);
		}
		else if(tagName == "group")
		{
			field = lookupXMLFldNumber(attName);
			dd.addMsgFlds(msgType, field, attIsReq == "Y");
			addXMLGroup(fldNode,msgType,*groupDD);
		}
		if(first == 0)
			first = field;
		fldNode = fldNode->next_sibling();
	}

	if(first)
		dd.addGroup(msgType,group, first, groupDD);

	return 0;
}



int IMIXCheck::lookupXMLFldNumber(const string &name)
{
	return m_mFldKeyName[name];
}

int IMIXCheck::lookupXMLFldNumber(rapidxml::xml_node<> *node)
{
	std::string name = node->first_attribute("name")->value();
	return m_mFldKeyName[name];
}

void IMIXCheck::addFlds(int num)
{
	m_sFlds.insert(num);
}

void IMIXCheck::addFldName(int num, std::string name)
{	
	if(m_mFldKeyName.insert(std::make_pair(name,num)).second == false)
	{
		//AfxMessageBox("有重复的field",MB_OK | MB_ICONWARNING);
		return;
	}
	m_mFldKeyNo[num] = name;

}

void IMIXCheck::addFldValue(int num, std::string value)
{
	m_mValueToFld[num].insert(value);
}

void IMIXCheck::addValueName( int field, const std::string& value, const std::string& name )
{
	m_mValueToName[std::make_pair(field, value)] = name;
}

int IMIXCheck::addMsgFlds(std::string msgType, int num, bool isReq)
{
	if(msgType == "header")
	{
		m_mHeadFlds[num] = isReq;
	}
	else if(msgType == "trailer")
	{
		m_mTrailFlds[num] = isReq;
	}
	if(isReq)
		m_mMsgFldsIsBool[num] = isReq;
	m_mMsgFlds[msgType].insert(num);
	return 0;
}

void IMIXCheck::addGroup(std::string msg,int field, int first, IMIXCheck *&dd)
{
	m_mGroup[std::make_pair( msg, field )] = std::make_pair(first,dd);//把msg下面的group放到map里边
}

void IMIXCheck::addHeaderFlds(int field,bool isReq)
{
	m_mHeadFlds[field] = isReq;
}

void IMIXCheck::addTrailerFlds(int field,bool isReq)
{
	m_mTrailFlds[field] = isReq;
}

void IMIXCheck::setExtraFld(int num)
{
	extraFld = num;
}

int IMIXCheck::getExtraFld(void)
{
	return extraFld;
}
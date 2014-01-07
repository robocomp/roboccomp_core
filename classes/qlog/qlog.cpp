/*
 *    Copyright (C) 2010 by RoboLab - University of Extremadura
 *
 *    This file is part of RoboComp
 *
 *    RoboComp is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    RoboComp is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RoboComp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "qlog.h"


qLog* qLog::logger = NULL;
qLog::qLog()
{
  log = "local"; 
}
qLog::~qLog(){
}

qLog* qLog::getInstance(){
	if(logger==NULL)
	    logger=new qLog();
	return logger;
}
void qLog::showConsole()
{
      printf("%s::%s::%s::%i::%s::%s::%s\n", timeStamp.c_str(), type.c_str(), file.c_str(), nLine, sender.c_str(), method.c_str(), message.c_str());
}
#if COMPILE_LOGGERCOMP==1
void qLog::sendLogger(){
	mess.sender = sender;
	mess.method = method;
	mess.file = file;
	mess.line = nLine;
	mess.timeStamp = timeStamp;
	mess.message = message;
	mess.type = type;	
	mess.fullpath = fullpath;
	try{
		prx->sendMessage(mess);
	}catch( const Ice::Exception& ex)
	{
		std::cout<<"Exception::Fail sending to Logger:"<<ex<<endl;
	}
}
void qLog::setProxy(std::string endpoint,RoboCompLogger::LoggerPrx _prx)
{
	log=QString(endpoint.c_str());
	if(endpoint=="logger" or endpoint=="both") //sender active
		prx = _prx;
}
#endif
void qLog::send(std::string _file, int line,std::string func,std::string strng,std::string _type)
{
	if (log=="none")
		return;
	else{
		//prepare message
		fullpath = _file;
		sender = "";
		QStringList list1 = QString::fromStdString(_file).split("/");
		sender = _file;
		if(QString::fromStdString(_file).contains("robocomp",Qt::CaseInsensitive) == true)
		{
			if(list1.contains("robocomp"))
			{
				if(list1.contains("Classes"))
					sender = list1[list1.indexOf("Classes")+1].toStdString();
				else if(list1.contains("HAL") or list1.contains("Essential"))
					sender = list1[list1.indexOf("HAL")+1].toStdString();
				else
					sender = list1[list1.indexOf("robocomp")+4].toStdString();
			}
		}
	    file = list1[list1.size()-1].toStdString();
	    method = func;
	    nLine = line;
	   // timeStamp = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss:zzz").toStdString();
		timeStamp = QDateTime::currentDateTime().toString("hh:mm:ss:zzz").toStdString();
	    message = strng;
	    type = _type;
	    fullpath=_file;
	    if(log=="local" or log=="both")
			showConsole();
	    if(log=="logger" or log=="both")
			#if COMPILE_LOGGERCOMP==1
				sendLogger();
			#else
				std::cout<<"Error component compiled without LoggerComp support, check CMAKELIST"<<endl;
			#endif
      }
}
void qLog::send(std::string file, int line,std::string func, QString strng,std::string type){
      send(file,line,func,strng.toStdString(),type);
}
void qLog::send(std::string file, int line,std::string func, const char* strng,std::string type){
      send(file,line,func,std::string(strng),type);
}



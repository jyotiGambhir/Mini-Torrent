#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <thread>
#include <pthread.h>
#define CHUNKSIZE 512
using namespace std;

struct csData{
	int port;
	//string filename;
};

struct threadData{
	int fd;
	string cport;
	string cip;
	
};

struct groupFileStruct
{
	string sha;
	string filesize;
	vector<string> clientWithFiles;
};

struct userInfoStruct
{
	string password;
	string clientport;
	string clientip;
	int flag;
};

struct groupFileUserStruct
{
	vector<pair<string,int> > fileslist;
	vector<string> userslist;
	string groupOwner;
	vector<string> pendingRequests;
};

unordered_map<string,struct groupFileStruct*> groupFileMap;
unordered_map<string,struct userInfoStruct*> userDetailsMap;
unordered_map<string,struct groupFileUserStruct*> groupFileUserMap;

vector<struct fileStruct*> fileDetails;

int checkUsernameExist(string user)
{
	if(userDetailsMap.find(user) != userDetailsMap.end())
		return 1;
	return 0;
}

int authenticate(string usr,string pwd)
{	cout<<"entered authenticate function"<<endl;
	cout<<"enetered username "<<usr<<endl;
	cout<<"entered pwd "<<pwd<<endl;
	if(userDetailsMap[usr]->password == pwd)
		return 1;
	return 0;
}

int checkGroupIdExist(string gid)
{
	if(groupFileUserMap.find(gid) != groupFileUserMap.end())
		return 1;
	return 0;
}

int userInGroup(string gid,string usr)
{
	if(find(groupFileUserMap.find(gid)->second->userslist.begin(),groupFileUserMap.find(gid)->second->userslist.end(),usr) != groupFileUserMap.find(gid)->second->userslist.end())
		return 1;
	return 0;
	
}

void printMapUserDetailsMap()
{
	for(unordered_map<string,struct userInfoStruct*>::iterator it=userDetailsMap.begin(); it!=userDetailsMap.end(); it++)
	{	cout<<"in map userDetailsMap "<<endl;
		cout<<"username is "<<it->first<<endl;
		cout<<"password is "<<(it->second)->password<<endl;
		cout<<"client port is "<<(it->second)->clientport<<endl;
		cout<<"client ip "<<(it->second)->clientip<<endl;
		cout<<"flag is "<<(it->second)->flag<<endl;
	}
}

void printgroupFileUserMap()
{	
	cout<<"in print groupFileUserMap"<<endl;
	for(unordered_map<string,struct groupFileUserStruct*>::iterator it=groupFileUserMap.begin(); it!=groupFileUserMap.end(); it++)
	{
		cout<<"gid id "<<it->first<<endl;
		cout<<"files list "<<endl;
		for(vector<pair<string,int>>::iterator p=groupFileUserMap[it->first]->fileslist.begin(); p!=groupFileUserMap[it->first]->fileslist.end(); p++)
			cout<<p->first<<" "<<p->second<<endl;
		cout<<"users list"<<endl;
		for(vector<string>::iterator p1=groupFileUserMap[it->first]->userslist.begin(); p1!=groupFileUserMap[it->first]->userslist.end(); p1++)
			cout<<*p1<<endl;
		cout<<"group owner is "<<groupFileUserMap[it->first]->groupOwner<<endl;
		for(vector<string>::iterator pp=groupFileUserMap[it->first]->pendingRequests.begin(); pp!=groupFileUserMap[it->first]->pendingRequests.end(); pp++)
			cout<<*pp<<endl;

	}

}

void printgroupFileMap()
{
	cout<<"in groupFileMap:"<<endl;
	for(unordered_map<string,struct groupFileStruct*>::iterator it=groupFileMap.begin(); it!=groupFileMap.end(); it++)
	{
		cout<<"group+filename "<<it->first<<endl;
		cout<<"sha "<<it->second->sha<<endl;
		cout<<"filesize "<<it->second->filesize<<endl;
		cout<<"list of clients"<<endl;
		for(vector<string>::iterator im=it->second->clientWithFiles.begin(); im!=it->second->clientWithFiles.end(); im++)
			cout<<*im<<endl;
	}
}

void* clientRequestServe(void* threadarg)
{	struct threadData *threadStruct;
	
	threadStruct=(struct threadData *)threadarg;
	int connectfd=threadStruct->fd;
	
	cout<<"accepted in server"<<endl;
	string usr="";
	while(1)
	{	
		char command[1024];
		cout<<"in sart of while(1)"<<endl;
		recv(connectfd,command,sizeof(command),0);
		cout<<"recived"<<endl;
		cout<<command<<endl;
		int ack=1;
		char* comm=strtok(command," ");
		if(comm==NULL)
		{
			pthread_exit(NULL);
		}
		
		if(strcmp(comm,"create_user") == 0)
		{	char* msg;
			cout<<"in create user"<<endl;
			string username=strtok(NULL," ");
			cout<<"usernameis "<<username<<endl;
			// check if username exists in map
			if(checkUsernameExist(username))
			{
				cout<<"Username already exists"<<endl;
				msg="Username already exists";
			}
			else
			{	//get password from command
				struct userInfoStruct* st=(struct userInfoStruct*)malloc(sizeof(struct userInfoStruct));
				st->password=strtok(NULL," ");
				cout<<"password is "<<st->password<<endl;
				//set flag when logged in
				st->flag=0;
				cout<<"fag is"<<st->flag<<endl;
				userDetailsMap.insert({username,st});
				cout<<"inserted"<<endl;
				msg="User Added Successfully";
			}
			printMapUserDetailsMap();
			cout<<"before send"<<endl;
			memset(command,'\0',sizeof(command));
			send(connectfd,msg,2048,0);
		}

		else if(strcmp(comm,"login")==0)
		{	char* msg;
			cout<<"in login command "<<endl;
			string recvUsername=strtok(NULL," ");
			string recvPasswd=strtok(NULL," ");
			//check for existence of username
			if(!checkUsernameExist(recvUsername))
			{
				cout<<"Username doesn't exist"<<endl;
				msg="Username doesn't exist";
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				continue;
			}
			//on authentication insert client ip and port and set flag
			if(authenticate(recvUsername,recvPasswd))
			{	usr=recvUsername;
				userDetailsMap[usr]->clientip=threadStruct->cip;
				userDetailsMap[usr]->clientport=threadStruct->cport;
				userDetailsMap[usr]->flag=1;
				msg="Authentication Successful";
			}
			else
			{
				cout<<"Authentication Failed : Password Incorrect "<<endl;
				msg="Authentication Failed : Password Incorrect";
			}
			printMapUserDetailsMap();
			//reset buffer
			memset(command,'\0',sizeof(command));
			send(connectfd,msg,2048,0);

		}

		else if(strcmp(comm,"create_group") == 0)
		{	char* msg;
			string gid=strtok(NULL," ");
			cout<<"gid is "<<gid<<endl;
			if(checkGroupIdExist(gid))
			{
				cout<<"Group Already Exists"<<endl;
				msg="Group Already Exists";
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				continue;
			}
			else
			{	cout<<"in create group"<<endl;
				struct groupFileUserStruct* st=(struct groupFileUserStruct*)malloc(sizeof(struct groupFileUserStruct));
				st->groupOwner=usr;
				st->userslist.push_back(usr);
				//groupFileUserMap[gid]=st;
				groupFileUserMap.insert({gid,st});
				cout<<"inserted"<<endl;
				printgroupFileUserMap();
				msg="Group Created Successfully";
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
			}
		}

		else if(strcmp(comm,"join_group")==0)
		{	char* msg;
			string gid=strtok(NULL," ");
			cout<<"join group req"<<endl;
			//check if group id doesn't exists
			if(!checkGroupIdExist(gid)){
				cout<<"groupid doesn't exist"<<endl;
				msg="Group Id Doesn't Exists";
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				continue;
			}
			else
			{	cout<<"raising req"<<endl;
				//find group id
				auto it=groupFileUserMap.find(gid);
				//push username in peding request
				it->second->pendingRequests.push_back(usr);
				msg="Request Pending with Owner";
				printgroupFileUserMap();
				memset(command,'\0',sizeof(command));
				//cout<<"sending msg"<<endl;
				send(connectfd,msg,2048,0);
				//cout<<"sent"<<endl;
			}
		}

		else if(strcmp(comm,"leave_group") == 0)
		{
			char* msg;
			string gid=strtok(NULL," ");
			if(!checkGroupIdExist(gid))
			{	msg="Group Id Doesn't Exists";
				// cout<<msg<<endl;
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				continue;
			}
			else
			{	//remove user from users list in groupFileUserMap
				auto it=groupFileUserMap.find(gid);
				auto itr=find(it->second->userslist.begin(),it->second->userslist.end(),usr);
				it->second->userslist.erase(itr);
				//remove user from groupFile Map
				for(unordered_map<string,struct groupFileStruct*>::iterator i=groupFileMap.begin(); i!=groupFileMap.end(); i++)
				{
					//if(groupFileMap->first == gid)
					auto pp=find(i->second->clientWithFiles.begin(),i->second->clientWithFiles.end(),usr);
					i->second->clientWithFiles.erase(pp);
				}
				printgroupFileUserMap();
				msg="Left Group Successfully";
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
			}
		}

		else if(strcmp(comm,"list_requests")==0)
		{	char* msg;
			string gid=strtok(NULL," ");
			cout<<"gid is "<<gid<<endl;
			if(!checkGroupIdExist(gid))
			{	msg="Group_Id_Doesn't_Exists";
				cout<<"msg is"<<msg<<endl;
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				recv(connectfd,&ack,sizeof(ack),0);
				//cout<<"ack"<<ack<<endl;
				msg="Group Id Doesn't Exists";
				//memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				//int m=23;
				//string msg1="hi";
				//send(connectfd,(char *)msg1.c_str(),sizeof(msg1),0);
				//cout<<msg1<<endl;
				//int by=recv(connectfd,&ack,sizeof(ack),0);
				//cout<<"by "<<by<<" ack "<<ack<<endl;
				continue;
			}


			else
			{	msg="Group_Id_Exists";
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				recv(connectfd,&ack,sizeof(ack),0);
				auto it=groupFileUserMap.find(gid);
				int mp=it->second->pendingRequests.size();
				cout<<"n is "<<mp<<endl;
				send(connectfd,&mp,sizeof(mp),0);
				// cout<<"find for the group"<<endl;
				//ack=2;
				recv(connectfd,&ack,sizeof(ack),0);
				string val;
				
				
				for(vector<string>::iterator ip=it->second->pendingRequests.begin(); ip!=it->second->pendingRequests.end(); ip++)
				{	val=*ip;
					send(connectfd,(char*)val.c_str(),val.length(),0);
					recv(connectfd,&ack,sizeof(ack),0);
				}
				//recv(connectfd,&ack,ack,0);

				send(connectfd,&ack,sizeof(ack),0);
				//recv(connectfd,&ack,ack,0);
				
			}
			

		}

		else if(strcmp(comm,"accept_request")==0)
		{	char* msg;
			cout<<"in accept request"<<endl;
			string gid=strtok(NULL," ");
			string adduser=strtok(NULL," ");
			if(!checkGroupIdExist(gid))
			{	msg="Group Id Doesn't Exists";
				// cout<<msg<<endl;
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				continue;
			}
			else
			{	msg="User Added To The Group";
				auto it=groupFileUserMap.find(gid);
				auto itr=find(it->second->pendingRequests.begin(),it->second->pendingRequests.end(),usr);
				it->second->pendingRequests.erase(itr);
				it->second->userslist.push_back(adduser);
				memset(command,'\0',sizeof(command));
				printgroupFileUserMap();
				send(connectfd,msg,2048,0);
			}


		}

		else if(strcmp(comm,"list_groups")==0)
		{	char* msg;
			cout<<"list groups command"<<endl;
			
				memset(command,'\0',sizeof(command));
				int n;
				n=groupFileUserMap.size();
				cout<<"n is"<<endl;
				send(connectfd,&n,sizeof(n),0);
				recv(connectfd,&ack,sizeof(ack),0);
				for(unordered_map<string,struct groupFileUserStruct*>::iterator it=groupFileUserMap.begin(); it!=groupFileUserMap.end(); it++)
				{	
					send(connectfd,(char*)it->first.c_str(),it->first.length(),0);
					recv(connectfd,&ack,sizeof(ack),0);
				}
				
				send(connectfd,&ack,sizeof(ack),0);
				
		}

		else if(strcmp(comm,"list_files")==0)
		{
			char* msg;
			cout<<"in list files"<<endl;
			string gid=strtok(NULL," ");
			
			if(!checkGroupIdExist(gid))
			{	msg="Group_Id_Doesn't_Exists";
				// cout<<msg<<endl;
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				continue;
			}
			else
			{
				msg="Group_Id_Exists";
				// cout<<msg<<endl;
				memset(command,'\0',sizeof(command));
				send(connectfd,msg,2048,0);
				int num=groupFileUserMap[gid]->fileslist.size();
				send(connectfd,&num,sizeof(num),0);
				for(int i=0; i<groupFileUserMap[gid]->fileslist.size();i++)
				{
					if(groupFileUserMap[gid]->fileslist[i].second==1)
					{	
						string st=groupFileUserMap[gid]->fileslist[i].first;
						send(connectfd,(char*)st.c_str(),sizeof(st),0);
					}
					else
					{
						string pp="ignore";
						send(connectfd,(char*)pp.c_str(),sizeof(pp),0);
					}
				}
				recv(connectfd,&ack,sizeof(ack),0);
			}

		}

		else if(strcmp(comm,"upload_file")==0)
		{	string filen;
			cout<<"in upload_file"<<endl;
			// cout<<command<<endl;
			struct groupFileStruct *fs=(struct groupFileStruct*)malloc(sizeof(struct groupFileStruct));
			filen=strtok(NULL," ");
			cout<<"filename is "<<filen<<endl;
			string gid=strtok(NULL," ");
			cout<<"gid is "<<gid<<endl;
			// fs->username=strtok(NULL," ");
			// cout<<"username "<<fs->username<<endl;
			//if(!gid=)
			//check for existnce of group
			int p=3;
			if(!checkGroupIdExist(gid)){
				p=1;
				cout<<"p sent "<<p<<endl;
				send(connectfd,&p,sizeof(p),0);
				continue;
			}
			else if(!userInGroup(gid,usr))
			{
				p=2;
				cout<<"p sent is "<<p<<endl;
				send(connectfd,&p,sizeof(p),0);
				continue;
			}
			else
			{	



				cout<<"p sent is "<<p<<endl;
				send(connectfd,&p,sizeof(p),0);
				fs->filesize=strtok(NULL, " ");
				cout<<"size is "<<fs->filesize<<endl;
				memset(command,'\0',sizeof(command));
				send(connectfd,&ack,ack,0);
				string shacal="";
				char bufsha[25]={'\0'};
				int n;
				cout<<"receiving sha"<<endl;
				while(n=recv(connectfd,bufsha,25,0)>0)
				{	cout<<"n is "<<n<<endl;
					if(strcmp(bufsha,"end")==0)
						break;
					shacal+=bufsha;
					cout<<shacal<<endl;
					send(connectfd,&ack,sizeof(ack),0);
				}
				cout<<"length of received sha "<<shacal.length()<<endl;
				cout<<shacal<<endl;
				fs->sha=shacal;
				fs->clientWithFiles.push_back(usr);
				cout<<"Done"<<endl;
				string hh=gid+filen;
				//groupFileMap.insert({hh,fs});
				//printgroupFileUserMap();
				
				int flagg=0;
				auto iptr=groupFileUserMap.find(gid);
				if(groupFileUserMap.find(gid)!=groupFileUserMap.end()){
					vector<pair<string,int>> vect=groupFileUserMap[gid]->fileslist;
					for(int i=0;i<vect.size();i++){
						if(vect[i].first==filen){
							vect[i].second=1;
							flagg=1;
							groupFileMap[hh]->clientWithFiles.push_back(usr);
							break;
						}
					}
					if(flagg==0){
						(groupFileUserMap[gid]->fileslist).push_back(make_pair(filen,1));
						groupFileMap.insert({hh,fs});
					}
				}
				// auto pptr=find(iptr->second->fileslist.begin(),iptr->second->fileslist.end(),filen);
				// if(pptr == iptr->second->fileslist.end())
				// 	iptr->second->fileslist.push_back(make_pair(filen,1));
				// else
				// 	pptr->second=1;
				printgroupFileUserMap();
				cout<<"Print group file map"<<endl;
				printgroupFileMap();
				send(connectfd,&ack,sizeof(ack),0);
				cout<<"completed uploaded"<<endl;
				// fs->clientip=
			}
		}
		else if(strcmp(comm,"download_file")==0)
		{	char* filename;
			char* gid;
			gid=strtok(NULL," ");
			filename=strtok(NULL," ");
			//recv(connectfd,&filename,sizeof(filename),0);
			cout<<filename<<" in serve request"<<endl;
			FILE *fp=fopen(filename,"rb");
			fseek(fp,0,SEEK_END);
			int size=ftell(fp);
			rewind(fp);
			send(connectfd,&size,sizeof(size),0);
			fclose(fp);
			string filenamestr=filename;
			string gidstr=gid;
			int no_of_chunks=ceil((float)size/CHUNKSIZE);
			int p=no_of_chunks;
			cout<<"p is"<<p<<endl;
			recv(connectfd,&ack,sizeof(ack),0);
			//string compstr=filename+gid;
			cout<<"ack received"<<endl;
			cout<<gidstr+filenamestr<<"combo is"<<endl;
			string send_sha=groupFileMap[gidstr+filenamestr]->sha;
			cout<<send_sha<<endl;
			int i=0;
			while(p--)
			{	cout<<"p is "<<p<<" i is "<<i<<endl;
				string val=send_sha.substr(i,20);
				cout<<"sending val"<<val<<endl;
				send(connectfd,(char*)val.c_str(),val.length(),0);
				i+=20;
				recv(connectfd,&ack,sizeof(ack),0);
			}
			vector<pair<string,string>> list_of_ip;
			cout<<"number of clients is "<<groupFileMap[gidstr+filenamestr]->clientWithFiles.size()<<endl;
			vector<string> list_of_clients=groupFileMap[gidstr+filenamestr]->clientWithFiles;
			for(int im=0; im<list_of_clients.size(); im++)
			{	
				cout<<list_of_clients[im]<<endl;
				if(userDetailsMap[list_of_clients[im]]->flag)
				{
					list_of_ip.push_back(make_pair(userDetailsMap[list_of_clients[im]]->clientport,userDetailsMap[list_of_clients[im]]->clientip));
				}
			}
			for(int ti=0; ti<list_of_ip.size(); ti++)
				cout<<list_of_ip[ti].first<<" "<<list_of_ip[ti].second<<endl;
			int num_of_clients=list_of_ip.size();
			cout<<num_of_clients<<"num_of_clients"<<endl;
			send(connectfd,&num_of_clients,sizeof(num_of_clients),0);
			//int pl=num_of_clients;
			recv(connectfd,&ack,sizeof(ack),0);
			string val1;
			for(int lp=0; lp<num_of_clients; lp++)
			{	
				
				val1=list_of_ip[lp].first+" "+list_of_ip[lp].second;
				cout<<val1<<endl;
				send(connectfd,(char*)val1.c_str(),val1.length(),0);
				recv(connectfd,&ack,sizeof(ack),0);
			}
			
		}
		else if(strcmp(comm,"logout")==0)
		{
			userDetailsMap[usr]->flag=0;
			send(connectfd,&ack,sizeof(ack),0);
		}
		cout<<"execute command"<<endl;
	}
	close(connectfd);
	
}


void* serverFunc(void* threadarg)
{	struct csData *d;
	d=(struct csData*)threadarg;
	cout<<"in server"<<endl;
	
	int socketfd=socket(PF_INET,SOCK_STREAM,0);
	
	if(socketfd<0)
	{
		perror("socket failed");
		pthread_exit(NULL);
	}
	struct sockaddr_in serveraddr;
	
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(d->port);
	
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	int addrlen=sizeof(serveraddr);
	if(bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("Error in Binding");
		pthread_exit(NULL);
	}
	cout<<"binded in server"<<endl;
	if(listen(socketfd,1000) < 0)
	{
		perror("ECONNREFUSED");
		pthread_exit(NULL);
	}
	cout<<"listening in server"<<endl;
	int connectfd;
	int i=0;
	pthread_t threadarr[1000];

	while((connectfd=accept(socketfd,(struct sockaddr*)&serveraddr,(socklen_t*)&addrlen)) > 0)
	{	cout<<"entered"<<endl;
		//struct threadData *th=(struct threadData*)malloc(sizeof(struct threadData));
		struct threadData *th=new threadData;
		th->fd=connectfd;
		char* ipclient=new char[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(serveraddr.sin_addr),ipclient,INET_ADDRSTRLEN);
		th->cip=ipclient;
		cout<<"sin port"<<serveraddr.sin_port<<endl;
		cout<<"ntohs "<<ntohs(serveraddr.sin_port)<<endl;
		th->cport=to_string(ntohs(serveraddr.sin_port));
		//th->cport="123";
		cout<<"ip "<<th->cip<<endl;
		cout<<"port"<<th->cport<<endl;
		//cout<<th.filename<<" craeting thread"<<endl;
			
		pthread_create(&threadarr[i],NULL,clientRequestServe,(void*)th);
		pthread_detach(threadarr[i]);
		i++;
		
	}
	
	close(socketfd);
	
}


int main()
{	
	pthread_t serverThread;
	cout<<"initialized"<<endl;
	struct csData serverDetails;
	cout<<"Port number for server"<<endl;
	cin>>serverDetails.port;
	
	pthread_create(&serverThread,NULL,serverFunc,(void*)&serverDetails);
	cout<<"server thread returned"<<endl;
	
	pthread_join(serverThread,NULL);
	
}
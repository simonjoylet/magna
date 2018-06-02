
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "definitions.h"
#include "endian.h"
#include "request.h"

#define NUM_OF_SERVERS 256
#define MAX_INT 0x7fffffff

int Server[NUM_OF_SERVERS];
int UniqueServers=0;

uint32_t TotalRequests=0;
double TotalBytes = 0.0;
uint32_t MinObject= MAX_INT;
uint32_t MaxObject=0;
uint32_t MinClient= MAX_INT;
uint32_t MaxClient=0;
uint32_t MinTime = MAX_INT;
uint32_t MaxTime = 0;
uint32_t MinSize = MAX_INT;
uint32_t MaxSize = 0;
uint32_t StartTime;
uint32_t LastTime = 0;
int OutOfOrder=0;
int Endian = NO_ENDIAN;

/**************************************************************************/
/* READ LOG                                                               */
/**************************************************************************/
const uint32_t SAMPLE_ROUND = 2500;
uint32_t myLogCount = 0;
uint32_t exactCount = 0;
uint32_t timeOffset = 0;
FILE * sampleFile = NULL; 
void ReadLog(const char * fileName)
{

	FILE * logFile = fopen(fileName, "rb");
	
  struct request BER, LER, *R;
  int value;
  int count=0;
  int raw, part1,part2;
  int region,server;
  int http, status;
  int method, type;
  time_t ts;
  struct tm *time_info;

  /* status indicator */
  fprintf(stderr,"Reading Access Log\n");

  /* read the initial request */
  if ((fread(&BER, sizeof(struct request), 1, logFile)) != 1)
    {
      fprintf(stderr,"Error: Failed to read initial request!\n");
      exit(0);
    }

  switch(Endian)
    {
    case LITTLE_ENDIAN:
      LittleEndianRequest(&BER, &LER);
      R = &LER;
      break;

    case BIG_ENDIAN:
      R = &BER;
      break;
      
    default:
      fprintf(stderr,"Error: unknown Endian!\n");
      exit(-1);
    }
	
  /* save start time */
  StartTime = R->timestamp;
  

  /* read through access log */
  while((!feof(logFile)) && (1))
    {
      /* status indicator */
      if(count % 1000000 == 0)
	fprintf(stderr,"%d\n", count);
      count++;

      /* update statistics */
      TotalRequests++;
      if(R->size != NO_SIZE)
	TotalBytes+= R->size;

	  

      /* check timestamp */
      if(R->timestamp < LastTime)
	OutOfOrder++;

      /* update minima and maxima */
      MinTime = MINIMUM(MinTime, R->timestamp);
      MaxTime = MAXIMUM(MaxTime, R->timestamp);
      MinObject = MINIMUM(MinObject, R->objectID);
      MaxObject = MAXIMUM(MaxObject, R->objectID);
      MinClient = MINIMUM(MinClient, R->clientID);
      MaxClient = MAXIMUM(MaxClient, R->clientID);
      if(R->size != NO_SIZE)
	{
	  MinSize = MINIMUM(MinSize, R->size);
	  MaxSize = MAXIMUM(MaxSize, R->size);
	}

      /* update last time */
      LastTime = R->timestamp;

	  // 检查时间戳
	  ts = (time_t)R->timestamp;
	  time_info = gmtime(&ts);
	  if (time_info->tm_mday == 22 && time_info->tm_hour >= 12)
	  {
		  if (myLogCount == 0)
		  {
			  timeOffset = R->timestamp;
		  }
		  
		  if (myLogCount % SAMPLE_ROUND == 0)
		  {
			  R->timestamp -= timeOffset;
			  fwrite(R, sizeof(struct request), 1, sampleFile);
			  ++exactCount;
		  }
		  ++myLogCount;
		  //fprintf(stderr, "%d\n", 11111111);
	  }

      /* determine the server and region */
      raw = (int)R->server;

      /* update server statistics */
      if((raw >=0) && (raw < NUM_OF_SERVERS))
	{
	  if(Server[raw] == 0)
	    UniqueServers++;
	  Server[raw]++;
	}
      else
	{
	  fprintf(stderr,"Error: invalid server info! %d\n", raw);
	  exit(0);
	}

      part1 = raw & 0xe0;
      part1 = part1 >> 5;
      part2 = raw & 0x1f;
      region = part1;
      server = part2;

      /* safety checks */
      if(region >= NUM_OF_REGIONS)
	{
	  fprintf(stderr,"Error: invalid region! %d\n", region);
	  exit(0);
	}

      if(server >= 32)
	{
	  fprintf(stderr,"Error: invalid server! %d\n", server);
	  exit(0);
	}

      /* determine http version and status */
      raw = (int)R->status;
      
      /* safety check */
      if((raw < 0) || (raw > 255))
	{
	  fprintf(stderr,"Error: invalid status info! %d\n", raw);
	  exit(0);
	}

      part1 = raw & 0xc0;
      part1 = part1 >> 6;
      part2 = raw & 0x3f;
      http = part1;
      status = part2;

      /* safety checks */
      if(http >= NUM_OF_PROTOCOLS)
	{
	  fprintf(stderr,"Error: invalid protocol! %d\n", http);
	  exit(0);
	}

      if(status >= NUM_OF_CODES)
	{
	  fprintf(stderr,"Error: invalid status code! %d\n", status);
	  exit(0);
	}

      /* determine the method */
      method = (int)R->method;

      /* safety check */
      if((method < 0) || (method >= NUM_OF_METHODS))
	{
	  fprintf(stderr,"Error: invalid method! %d\n", method);
	  exit(0);
	}

      /* determine the filetype */
      type = (int)R->type;

      /* safety check */
      if((type < 0) || (type >= NUM_OF_FILETYPES))
	{
	  fprintf(stderr,"Error: invalid filetype! %d\n", type);
	  exit(0);
	}

      /* read the next request */
      value = fread(&BER, sizeof(struct request), 1, logFile);

      if(value == 1)
	{
	  switch(Endian)
	    {
	    case LITTLE_ENDIAN:
	      LittleEndianRequest(&BER, &LER);
	      R = &LER;
	      break;
	      
	    case BIG_ENDIAN:
	      R = &BER;
	      break;
	      
	    default:
	      fprintf(stderr,"Error: unknown Endian!\n");
	      exit(-1);
	    }
	}
    }

  /* final count */
  fprintf(stderr,"%d\n", count);

  fprintf(stderr, "----------------%d %d------------------\n", myLogCount, exactCount);

}

void PrintResults()
{
  int i;
  uint32_t total=0;
  int unique = 0;

  /* status indicator */
  fprintf(stderr,"Printing Results\n");

  fprintf(stdout,"    Total Requests: %u\n", TotalRequests);
  fprintf(stdout,"       Total Bytes: %.0f\n", TotalBytes);
  fprintf(stdout,"Mean Transfer Size: %f\n", 
	  TotalBytes/(double)TotalRequests);
  fprintf(stdout,"     Min Client ID: %u\n", MinClient);
  fprintf(stdout,"     Max Client ID: %u\n", MaxClient);
  fprintf(stdout,"     Min Object ID: %u\n", MinObject);
  fprintf(stdout,"     Max Object ID: %u\n", MaxObject);
  fprintf(stdout,"          Min Size: %u\n", MinSize);
  fprintf(stdout,"          Max Size: %u\n", MaxSize);
  fprintf(stdout,"          Min Time: %u\n", MinTime);
  fprintf(stdout,"          Max Time: %u\n", MaxTime);

  fprintf(stdout,"        Start Time: %u\n", StartTime);
  fprintf(stdout,"       Finish Time: %u\n", LastTime);
  fprintf(stdout,"      Out of Order: %d\n", OutOfOrder);

  fprintf(stdout,"    Unique Servers: %d\n", UniqueServers);
  
  for(i=0; i<NUM_OF_SERVERS; i++)
    if(Server[i] > 0)
      {
	fprintf(stdout,"Server %d: %d\n", i, Server[i]);
	total += Server[i];
	unique++;
      }
  fprintf(stdout,"Check:\n");
  fprintf(stdout,"Total Requests: %u\n", total);
  fprintf(stdout,"Unique Servers: %d\n", unique);
}

void Initialize(int argc, char **argv)
{
  int i;

  /* status indicator */
  fprintf(stderr,"Initializing\n");

  /* determine which endian platform uses */
  Endian = CheckEndian();

  /* initialize server statistics */
  for(i=0; i<NUM_OF_SERVERS; i++)
    Server[i]=0;
}

void Terminate()
{
  /* status indicator */
  fprintf(stderr,"Terminating\n");
}

uint32_t AnalyzeLog(const char * fileName)
{
	FILE * logFile = fopen(fileName, "rb");
	struct request req;
	uint32_t reqCount = 0;
	uint32_t countArray[72] = {0};
	uint32_t typeArray[10] = { 0 };
	while ((!feof(logFile)) && (1))
	{
		int value = fread(&req, sizeof(struct request), 1, logFile);
		countArray[req.timestamp / 1000] += 1;
		typeArray[req.type % 10] += 1;
		++reqCount;
	}

	FILE * countFile = fopen("count.txt", "wt");
	for (int i = 0; i < 72; ++i)
	{
		char tmp[32] = { 0 };
		sprintf(tmp, "%d\n", countArray[i]);
		fwrite(tmp, sizeof(tmp), 1, countFile);
	}

	fclose(logFile);
	fclose(countFile);
	return reqCount;
}

typedef struct 
{
	uint32_t id; // 请求id
	uint32_t interval; // 访问时间间隔，单位us
	char service[32]; // 服务名称
	uint32_t weight; // 请求权重
}AppReq;

void GenerateTrafficFile(const char * logFileName, const char * trafficFileName, uint32_t totalCount)
{
	FILE * logFile = fopen(logFileName, "rb");
	FILE * trafficFile = fopen(trafficFileName, "wb");
	fwrite(&totalCount, sizeof(totalCount), 1, trafficFile);
	const char * services[] = { "Comp_1", "Comp_1", "Comp_1",
		"Comp_2", 
		"Comp_3", "Comp_3", "Comp_3", "Comp_3", "Comp_3", "Comp_3" };
	struct request req;
	uint32_t reqCount = 0;
	uint32_t lastLogTime = 0;
	while ((!feof(logFile)) && (1))
	{
		++reqCount;
		int value = fread(&req, sizeof(struct request), 1, logFile);
		AppReq ar = {0};
		ar.id = reqCount;
		ar.interval = 1000 * (req.timestamp - lastLogTime);
		strcpy(ar.service, services[rand()%10]);
		ar.weight = req.clientID % 3 + 1;
		fwrite(&ar, sizeof(ar), 1, trafficFile);
		lastLogTime = req.timestamp;
	}

	fclose(logFile);
	fclose(trafficFile);
}

typedef struct
{
	uint32_t reqId;
	char serviceName[16];
	uint32_t clientWeight;
	uint64_t localBegin;
	uint64_t localEnd;
	uint32_t compLamda;
	uint32_t queueLength;
	uint32_t queueTime;
	uint32_t processTime;

}ReqLog;

typedef struct 
{
	uint32_t sendId; // 当前正在发送的id
	uint32_t sendLamda; // 当前的发送lamda
	double cpuLoad;
	double diskLoad;
}LoadLog;

double GetSatisfaction(double totalTime, uint32_t weight)
{
	if (totalTime <= 500)
	{
		return 4 * weight;
	}
	else if (totalTime <= 1000)
	{
		return 2 * weight;
	}
	return 0;
}

void AnalyzeStress(const char * logFileName, const char * stressFileName, const char * outFileName)
{
	FILE * stressFile = fopen(stressFileName, "rb");
	
	// 读取前面的数据，直接舍弃
	uint32_t loadLogCount = 0;
	fread(&loadLogCount, sizeof(loadLogCount), 1, stressFile);
	for (size_t i = 0; i < loadLogCount; i++)
	{
		LoadLog log;
		fread(&log, sizeof(LoadLog), 1, stressFile);
	}
	
	uint32_t reqLogCount = 0;
	fread(&reqLogCount, sizeof(reqLogCount), 1, stressFile);



	// 打开日志文件
	FILE * logFile = fopen(logFileName, "rb");
	
	struct request req;
	ReqLog reqlog;
	uint32_t reqCount = 0;
	double countArray[72] = { 0 };
	while ((!feof(logFile)) && (1))
	{
		++reqCount;
		fread(&reqlog, sizeof(ReqLog), 1, stressFile);
		double su = GetSatisfaction(reqlog.localEnd - reqlog.localBegin, reqlog.clientWeight);

		fread(&req, sizeof(struct request), 1, logFile);
		countArray[req.timestamp / 600] += su;
	}

	// 将满意度效用写入结果文件
	FILE * suFile = fopen(outFileName, "wt");
	for (int i = 0; i < 72; ++i)
	{
		char tmp[32] = { 0 };
		sprintf(tmp, "%.2f\n", countArray[i]);
		fwrite(tmp, sizeof(tmp), 1, suFile);
	}

	fclose(logFile);
	fclose(stressFile);
	fclose(suFile);
	return reqCount;
}

/**************************************************************************/
/* MAIN PROGRAM                                                           */
/**************************************************************************/
int main(int argc, char **argv)
{
  Initialize(argc, argv);

     const char * sampleFileName = "sample.log";
//   sampleFile = fopen(sampleFileName, "wb");
//   for (int i = 2; i < 8; ++i)
//   {
// 	  char fileName[64] = {0};
// 	  sprintf(fileName, "wc_day58_%d", i);
// 	  ReadLog(fileName);
//   }
//   fclose(sampleFile);

//   uint32_t reqCount = AnalyzeLog(sampleFileName);
// 
//   const char * trafficFileName = "worldcup.dat";
//   GenerateTrafficFile(sampleFileName, trafficFileName, reqCount);

	 AnalyzeStress(sampleFileName, "simu_worldcup.stress", "satisUtility.txt");
	 AnalyzeStress(sampleFileName, "simu_worldcup_tradition.stress", "satisUtility_tradition.txt");
	 AnalyzeStress(sampleFileName, "simu_worldcup_vectordot.stress", "satisUtility_vectordot.txt");
	 AnalyzeStress(sampleFileName, "simu_worldcup_optimal.stress", "satisUtility_optimal.txt");


  PrintResults();

  Terminate();

  getchar();
  return(1);
}

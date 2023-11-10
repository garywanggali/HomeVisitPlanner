#include<iostream>
#include<cmath>
#include<curl/curl.h>
#include<rapidjson/document.h>
#include<sstream>
#include<string>
#include<vector>
#include"csv.h"
#include<map>

using namespace std;
using namespace rapidjson;

int numstu,numtea=9,numclass,ans[1001],s=0;
double ygx=120.056144,ygy=30.281064;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
size_t totalSize = size * nmemb;
userp->append((char*)contents, totalSize);
return totalSize;
}

vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

struct point {
    double x,y,angle;
    string stuclass;
    bool visited = false;
    string address;
};
vector<point> points;

struct classes {
    int stucla,teacla;
}classes[1001];

struct group {
    int stugr,teagr;
}groups[1001];


double cdistance (double pfx,double ptx,double pfy,double pty) {
    double xx=pow(abs(pfx-ptx),2);
    double yy=pow(abs(pfy-pty),2);
    double distance=sqrt(xx+yy);
    return distance;
}

double cangle (double x, double y){
    double jangle=0;
    x-=ygx;
    y-=ygy;
    if (y==0 && x>0) {
        jangle=0.00;
    }else if (y==0 && x==0) {
        jangle=0.00;
    }else if (y==0 && x<0) {
        jangle= acos(-1);    
    }else if (y>0 && x==0) {
        jangle=acos(-1)/2;
    }else if (y<0 && x==0) {
        jangle = acos(-1)/2*3 ;   
    }else if (y>0 && x>0){
        jangle=atan(y/x);
    }else if (y<0 && x<0) {
        jangle=acos(-1)+atan(y/x); 
    }else if (y>0 && x<0) {
        jangle=-1*(atan(y/x))+acos(-1)/2;    
    }else if (y<0 && x>0) {
        jangle=-1*(atan(y/x))+ acos(-1)/2*3;    
    }            
    jangle = jangle*180/acos(-1);
    int angle2 = int(jangle);
    angle2=angle2%360;
    return angle2;
}

struct nearest 
{
    double shortest=1001;
    int stpx,stpy,rank;
};

nearest tdistance (double stx,double sty,double stp,double enp) {
    nearest near;
    near.rank=0;
    near.shortest=1001;
    for (int i=stp;i<enp;i++) {
        if (points[i].visited==false) 
        {
            double distance=cdistance(stx,points[i].x,sty,points[i].y);   
            if (near.shortest>distance) {
                near.shortest=distance;    
                near.stpx=points[i].x;
                near.stpy=points[i].y;
                near.rank=i;
            }
            //cout<<points[i].x<<" "<<points[i].y<<" "<<distance<<" "<<rank<<endl;
        } 
    }
    points[near.rank].visited=true;
    return near;
}

void get (string address,string CLASS) {
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    string response;

    curl = curl_easy_init();
    if(curl) {
        string url = "https://restapi.amap.com/v3/geocode/geo?key=e14e6e5100d26e13b1e729ad96711f45&address=";
        url += address;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();    
    //cout<<response<<endl;
    Document d;
    d.Parse(response.data());

    if(d.HasParseError()) {
        cout<< response<< endl;
        cerr << "Failed to parse JSON!" << endl;
        return ;
    }

    const Value& loc = d["geocodes"][0]["location"];
    //printf("\n\n loc:%s \n\n", loc.GetString());
    string str = loc.GetString();
    vector<string> tokens = split(str, ',');
    point p;
    p.x=stod(tokens[0]);
    p.y=stod(tokens[1]);
    p.address = address;
    p.stuclass=CLASS;
    points.push_back(p);
    // cout << "经度: " << loc["lng"].GetDouble() << ", 纬度: " << loc["lat"].GetDouble() << endl;
}

int main(int argc, char* argv[]) 
{
    
    if (argc<2) {
        cout<<"You should input file name!"<<endl;
        return 0;
    }
    io::CSVReader<3> in(argv[1]);
    in.read_header(io::ignore_extra_column, "no", "address","CLASS");
    int no; 
    string address; 
    string CLASS;
    int ll=0;
    map<string,int> incla;
    while(in.read_row(no, address,CLASS)){
        ll++;
        //cout << "No." << ll << ": " << address <<endl;
        incla[CLASS]=0;
        get(address,CLASS);
    }
    ll=0;
    for(auto it=incla.begin();it!=incla.end();++it) {
        (*it).second=ll;
        ll++;
    }
    // for (auto it=incla.begin();it!=incla.end();++it)
    // {
    //     cout << (*it).first << " " << (*it).second << endl;
    // }
    // for(int i=0;i<points.size();i++) {
    //     cout<<points[i].address<<":x="<<points[i].x<<", y="<<points[i].y<<endl;
    // }
    
    numstu=ll;
    cout<<numstu<<endl;
    
    //给每个点赋予属性
    for (int i=0;i<numstu;i++) {
        points[i].angle=cangle(points[i].x,points[i].y);
    }
    //给每个班赋予属性
    for (int i=0;i<numclass;i++) {
        int yu=numstu%numclass;
        classes[i].stucla=numstu/numclass;
        if (i<yu) {
            classes[i].stucla++;    
        }
        yu=numtea%numclass;
        classes[i].teacla=numtea/numclass;
        if (i<yu) {
            classes[i].teacla++;    
        }
    }
    for (int i=0;i<numtea;i++) {
        int yu;
        yu=numstu%numtea;
        groups[i].stugr=numstu/numtea;
        if (i<yu) {
            groups[i].stugr++;  
        }
        //cout<<groups[i].stugr<<endl;
    }
    //按照班级排序
    for (int i=0;i<numstu-1;i++) {
        for (int j=0;j<numstu-1-i;j++) {
            if (points[j].stuclass>points[j+1].stuclass) {
                swap(points[j],points[j+1]);    
            }
        }
    }
    //每个班内按照角度排序
    for (int f=0;f<numclass;f++) {
        int sth=0;
        for (int i=0;i<f;i++) {
            sth=sth+classes[i].stucla;    
        }
        for (int i=0;i<classes[f].stucla-1;i++) {
            for (int j=0;j<classes[f].stucla-1-i;j++) {
                if (points[j+sth].angle>points[j+1+sth].angle) {
                    swap(points[j+sth],points[j+1+sth]);    
                }
            }
        }    
    }


    
    for (int i=0;i<numtea;i++)
    {
        int sth=0;
        for (int j=0;j<i;j++)
        {
            sth+=groups[j].stugr;
        }
        int tx=ygx,ty=ygy;
        for (int j=0;j<groups[i].stugr;j++)
        {
            nearest a=tdistance(tx,ty,sth,sth+groups[j].stugr);
            tx=a.stpx;
            ty=a.stpy;
            cout<<"Teacher"<<i<<": "<<a.rank<<" "<<a.shortest<<endl;
        }
    }
    // for (int i=0;i<numstu;i++)
    // {
    //     cout<<points[i].x<<" "<<points[i].y<<endl;    
    // }
    return 0;
}


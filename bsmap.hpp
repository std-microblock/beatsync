#ifndef BSMAP_H
#define BSMAP_H
#include<QList>
#include"global.h"
#include <QDebug>

#define mMap(a,b) tmp.b=splited[a]

class BSDifficulty{
public:
    double notes,bombs,stars,nps,seconds;
};

#define minMaxJudge(filter,value) ((filter["min"].is_number()?\
    filter["min"].get<double>()<=value:1)&&\
    (filter["max"].is_number()?\
    filter["max"].get<double>()>=value:1))

class BSMap
{
public:
    QList<BSDifficulty> difficulties;
    QString id,name,url;
    bool ranked;

    bool matchJSONFilter(nljson filter){
        if(filter["stars"].is_object()&&!ranked)return 0;
        bool flagMatchBucket[100]={};
        for(int x=0;x<this->difficulties.length();x++){
            if(minMaxJudge(filter["nps"],difficulties[x].nps)){
                flagMatchBucket[0]=1;

            }
            if(minMaxJudge(filter["stars"],difficulties[x].stars)){
                flagMatchBucket[1]=1;

            }
        }
        bool flagPass=true;
        for(int x=0;x<2;x++)flagPass=flagPass&&flagMatchBucket[x];
        if(!flagPass)return false;
        return true;
    }
    static BSMap fromCSV(QString csv){
        BSMap tmp;
        auto splited=csv.split(',');
        if(splited.length()<6)return tmp;
        mMap(0,id);
        mMap(1,name);
        tmp.ranked=(splited[2]=="1");
        mMap(4,url);
        for(int x=5;x<splited.length();x++){
            auto diff=splited[x].split('~');
            if(diff.length()<5)continue;
            tmp.difficulties.push_front(
                        BSDifficulty{
                            diff[0].toDouble(),
                            diff[1].toDouble(),
                            diff[2].toDouble(),
                            diff[3].toDouble(),
                            diff[4].toDouble()});
        }
        return tmp;
    }
    static QList<BSMap> fromCSVMaps(QString* csv){
        QList<BSMap> tmp;
        auto spl=csv->split('\n');
        for(auto line=0;line<spl.length();line++){
            tmp.push_back(fromCSV(spl[line]));
        }
        return tmp;
    }
    static QList<BSMap> fromCSVMapsFast(QString* csvaddr){
        QList<BSMap> tmp;
        //        if (FILE *fp = fopen(csvaddr->toStdString().c_str(), "r")) {
        //            bool c=1;
        //            while(c){
        //            std::string id(30, '\0'),name(30, '\0'),ranked(30, '\0'),ranked2(30, '\0'),url(30, '\0');
        //            c=fscanf(fp, "%[^,]%[^,],%[^,],%[^,],%[^,]", id.data(),name.data(),ranked.data(),ranked2.data(),url.data());
        //#define resize(a) a.resize(strlen(a.data()));
        //                resize(id);resize(name);resize(ranked);resize(ranked2);resize(url);
        //                BSMap tmpMap;
        ////                if(url.size()!=0)
        //                qDebug()<<QString::fromStdString(id);
        //                #define tmpDStr(a,b) tmpMap.a=QString::fromStdString(rows[b])
        //            }
        //            fclose(fp);
        //        }

        //        rapidcsv::Document doc(csvaddr->toStdString(), rapidcsv::LabelParams(-1, -1));
        //        for(int x=0;x<doc.GetRowCount();x++){
        //            std::vector<std::string> rows = doc.GetRow<std::string>(x);


        //            tmpDStr(id,0);
        //            tmpDStr(name,1);
        //            tmpMap.ranked=(rows[2]=="1");
        //            tmpDStr(url,4);
        //            for(int x=5;x<rows.size();x++){
        //                auto diff=QString::fromStdString(rows[x]).split('~');
        //                if(diff.length()<5)continue;
        //                tmpMap.difficulties.push_front(
        //                            BSDifficulty{
        //                                diff[0].toDouble(),
        //                                diff[1].toDouble(),
        //                                diff[2].toDouble(),
        //                                diff[3].toDouble(),
        //                                diff[4].toDouble()});
        //            }
        //            tmp.push_back(tmpMap);
        //        }

        return tmp;
    }
    BSMap(){

    };
};

#endif // BSMAP_H

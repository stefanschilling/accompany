#include <MyTracker.h>

#include <list>
#include <iterator>
#include <set>
using namespace std;

MyTracker::MyTracker()
{
  id=0;
}

void MyTracker::trackHumans(const accompany_uva_msg::HumanLocations::ConstPtr& humanLocations)
{
  set<int> assigned;
  // all new locations
  for (unsigned int i=0;i<humanLocations->locations.size();i++)
  {
    // all known locations
    list<accompany_uva_msg::TrackedHuman >::iterator best;
    double maxDist=1;
    double bestDist=maxDist;
    for (list<accompany_uva_msg::TrackedHuman>::iterator it=trackedHumansList.begin();it!=trackedHumansList.end();it++)
    {
      double dx=humanLocations->locations[i].point.x-it->location.point.x;
      double dy=humanLocations->locations[i].point.y-it->location.point.y;
      double dz=humanLocations->locations[i].point.z-it->location.point.z;
      double squaredDistance=dx*dx+dy*dy+dz*dz;
      //cout<<"squaredDistance: "<<squaredDistance<<endl;
      if (squaredDistance<bestDist)
      {
        bestDist=squaredDistance;
        best=it;
      }
    }
    if (bestDist<maxDist) // if match found assign to best
    {
      //cout<<"match found"<<endl;
      assigned.insert(i);
      update(humanLocations,i,best);
    }
    //else
    //cout<<"no match found"<<endl;
  }

  // if no match found create new track
  for (unsigned int i=0;i<humanLocations->locations.size();i++)
  {
    if (assigned.find(i)==assigned.end()) // if not assigned
    {
      add(humanLocations,i);
    }
  }
}

accompany_uva_msg::TrackedHumans MyTracker::getTrackedHumans()
{
  removeOldTracks();
  ros::Time now=ros::Time::now();
  accompany_uva_msg::TrackedHumans trackedHumans;
  for (list<accompany_uva_msg::TrackedHuman>::iterator it=trackedHumansList.begin();it!=trackedHumansList.end();it++)
  {
    ros::Duration duration=now-(it->lastSeen);
    if (duration.toSec()<0.5) // only add when seen recently
    {
      trackedHumans.trackedHumans.push_back(*it);
    }
  }
  return trackedHumans;
}

int MyTracker::getID()
{
  return id++;
}

void MyTracker::removeOldTracks()
{
  ros::Time now=ros::Time::now();
  for (list<accompany_uva_msg::TrackedHuman>::iterator it=trackedHumansList.begin();it!=trackedHumansList.end();)
  {
    ros::Duration duration=now-(it->lastSeen);
    //cout<<"old: "<<duration.toSec()<<"  ";
    if (duration.toSec()>=2) // if 2 or more seconds old, remove
    {
      //cout<<"remove"<<endl;
      it=trackedHumansList.erase(it);
    }
    else
    {
      //cout<<"keep"<<endl;
      it++;
    }
  }
}

void MyTracker::update(const accompany_uva_msg::HumanLocations::ConstPtr& humanLocations,int index,
                       std::list<accompany_uva_msg::TrackedHuman>::iterator it)
{
  it->location=humanLocations->locations[index];
  it->lastSeen=ros::Time::now();
}

void MyTracker::add(const accompany_uva_msg::HumanLocations::ConstPtr& humanLocations,int index)
{
  accompany_uva_msg::TrackedHuman trackedHuman;
  trackedHuman.id=getID();
  trackedHuman.location=humanLocations->locations[index];
  trackedHuman.firstSeen=ros::Time::now();
  trackedHuman.lastSeen=trackedHuman.firstSeen;
  trackedHumansList.push_back(trackedHuman);
}

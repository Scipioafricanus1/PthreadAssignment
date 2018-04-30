#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <signal.h>
#include <algorithm>

using namespace std;


static int whittierBound = 0;
static int bearValleyBound = 0;
static int maxNCars;
static int bearValleyNumCarsCrossed,whittierNumCarsCrossed, currentNumCars, numCarsWaited = 0;
static string tunnelStatus = "Whittier";
static pthread_mutex_t mutex;
static pthread_cond_t ready = PTHREAD_COND_INITIALIZER;

struct carThread {
    unsigned travelTime;
    unsigned carNo;
    string bound;
};

class Cars {
private:
    unsigned arrivalTime;
    string bound;
    unsigned travelTime;
public:
    Cars(unsigned _arrivalTime, string _bound, unsigned _travelTime) {
        arrivalTime = _arrivalTime;
        bound = _bound;
        travelTime = _travelTime;
    }
    unsigned getArrivalTime() const { return arrivalTime; }
    string getBound() const { return bound; }
    unsigned getTravelTime() const { return arrivalTime; }
};
vector<Cars> carsTable;

void *bearValley(void *arg)
{
    pthread_mutex_lock(&mutex);

    struct carThread *carInfo;
    carInfo = (struct carThread *) arg;
    cout << "Bear Valley-bound Car # " << carInfo->carNo+1 << " arrives at tunnel." << endl;
    if (currentNumCars >= maxNCars) {
        numCarsWaited++;
    }
    while(tunnelStatus != "Bear Valley" || currentNumCars >= maxNCars) {
        pthread_cond_wait(&ready, &mutex);
    }

    cout << "Bear Valley-bound Car # " << carInfo->carNo+1 << " enters tunnel." << endl;
    currentNumCars++;

    pthread_mutex_unlock(&mutex);

    sleep(carInfo->travelTime);

    pthread_mutex_lock(&mutex);
    cout << "Bear Valley-bound Car # " << carInfo->carNo+1 << " leaves tunnel."<< endl;
    currentNumCars--;
    pthread_cond_broadcast(&ready);
    pthread_mutex_unlock(&mutex);
} // car
void *whittier(void *arg)
{
    pthread_mutex_lock(&mutex);
    struct carThread *carInfo;
    carInfo = (struct carThread *) arg;
    cout << "Whitter-bound Car # " << carInfo->carNo+1 << " arrives at tunnel." << endl;
    if (currentNumCars >= maxNCars) {
        numCarsWaited++;
    }
    while(tunnelStatus != "Whittier" || currentNumCars >= maxNCars) {
        pthread_cond_wait(&ready, &mutex);
    }


    cout << "Whitter-bound Car # " << carInfo->carNo+1 << " enters tunnel." << endl;
    currentNumCars++;
    pthread_mutex_unlock(&mutex);

    sleep(carInfo->travelTime);

    pthread_mutex_lock(&mutex);
    cout << "Whittier-bound Car # " << carInfo->carNo+1 << " leaves tunnel."<< endl;
    currentNumCars--;
    pthread_cond_broadcast(&ready);
    pthread_mutex_unlock(&mutex);
} // car

void *tunnelThread(void *args) {
    //tunnel cycles through 4 choices.

    while(true) {
        if(tunnelStatus == "Whittier") {
            cout <<"Whittier-bound traffic only" << endl;
            pthread_cond_broadcast(&ready);
            sleep(5);
            tunnelStatus = "No traffic WB";
            cout << "No traffic allowed" << endl;
        } else if (tunnelStatus == "No traffic WB") {
            sleep(5);
            tunnelStatus = "Bear Valley";
            cout << "Bear Valley-bound traffic only" << endl;
            pthread_cond_broadcast(&ready);
        } else if (tunnelStatus == "Bear Valley") {
            sleep(5);
            tunnelStatus = "No traffic BV";
            cout << "No traffic allowed" << endl;
        } else if(tunnelStatus == "No traffic BV") {
            sleep(5);
            tunnelStatus = "Whittier";
        }
    }

}

string delWhiteSpaces(string &str)  //function from ddacot of stackoverflow. Removes spaces from string.
{                                   //Modified it to make it work better for the assignment.
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
    str.erase(remove(str.begin(),str.end(), '\n'), str.end());
    str.erase(remove(str.begin(),str.end(),'\t'), str.end());
    return str;
}


int main() {
    cin >> maxNCars;
    string line;
    int count = 0;
//    while (getline (cin, line)) {
//        if(count == 0)
//        {
//            count++;
//            continue;
//        }
//        carsTable.push_back(line);
//    }
    unsigned arrivalTime;
    string bound;
    unsigned travelTime;
    while(cin >> arrivalTime >> bound >> travelTime) {
        carsTable.push_back(Cars(arrivalTime, bound, travelTime));
    }
    pthread_t tunnelTid;

    int const numCars = carsTable.size();
    pthread_mutex_init(&mutex, nullptr);
    pthread_t cars[numCars];
    struct carThread td[numCars];
    pthread_create(&tunnelTid, nullptr, tunnelThread, (void *) nullptr);



    for (unsigned i = 0; i < carsTable.size(); ++i) { //cycles through the input and creates threads after sanitizing inputs.
        td[i].carNo = i;
        td[i].travelTime = carsTable[i].getTravelTime();
        string s = carsTable[i].getBound();

        td[i].bound = delWhiteSpaces(s); //delWhiteSpces function just in case, but probably not needed.

        sleep(arrivalTime);

        if("WB" == s) {
            whittierBound++;
            pthread_create(&cars[i], nullptr, whittier, (void *) &td[i]);
        } else if("BB" == s) {
            bearValleyBound++;
            pthread_create(&cars[i], nullptr, bearValley, (void *) &td[i]);
        }
    }
    for (unsigned i = 0; i < carsTable.size(); i++) {
        pthread_join(cars[i], nullptr);
    }

    cout<< "Total # of Bear Valley-bound cars that arrived: " << bearValleyBound << endl;
    cout<< "Total # of Whittier-bound cars that arrived: " << whittierBound << endl;

    cout<< "Total # of cars that waited: " << numCarsWaited << endl;

    return 0;
}
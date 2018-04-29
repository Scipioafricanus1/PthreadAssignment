#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
#include <zconf.h>


using namespace std;


static int whittierBound;
static int bearValleyBound;
static int maxNCars;
static int bearValleyNumCarsCrossed,whittierNumCarsCrossed, currentNumCars, numCarsWaited = 0;
static int carNo;
static string tunnelStatus = "Whittier";

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

void *car(void *arg)
{
    struct carThread *carInfo;
    while(tunnelStatus != carInfo->bound);
    carInfo = (struct carThread *) arg;
    cout << "Car number " << carInfo->carNo << " enters tunnel.";
    sleep(carInfo->travelTime);
} // car

void *tunnelThread(void *arg) {
    //tunnel first cycles through 5 choices.
    while(true) {
        if(tunnelStatus == "Whittier") {
            cout <<"Whittier-bound traffic only" << endl;
            sleep(5);
            tunnelStatus = "No traffic WB";
            cout << "No traffic allowed" << endl;
        } else if (tunnelStatus == "No traffic WB") {
            sleep(5);
            tunnelStatus = "Bear Valley";
            cout << "Bear Valley-bound traffic only" << endl;
        } else if (tunnelStatus == "Bear Valley") {
            sleep(5);
            tunnelStatus = "No traffic BV";
            cout << "No traffic allowed" << endl;
        } else if (tunnelStatus == "No traffic BV") {
            sleep(5);
            tunnelStatus = "Whittier";
        }
    }

}

void input() {
    cin >> maxNCars;
    cout << "Total # of cars is: " << maxNCars << endl;
    unsigned arrivalTime, travelTime;
    string bound;
    while(cin >> arrivalTime >> bound >> travelTime) {
        if(bound == "WB") {
            bound = "Whittier";
        }
        else if (bound == "BB") {
            bound = "Bear Valley";
        }
        carsTable.emplace_back(Cars(arrivalTime, bound, travelTime));
    }
}


int main() {
    input();
    pthread_t tunnelTid;
    pthread_t *cars;
    cars = new (nothrow) pthread_t[maxNCars];
    if (cars == nullptr)
        cout << "Error: mem couldn't be allocated";
    else {
        pthread_create(&tunnelTid, nullptr, tunnelThread, (void *) nullptr);
        struct carThread *td;
        td = new (nothrow) struct carThread[maxNCars];
        if(td == nullptr) {
            cout << "Error: memory couldn't be allocated";
        } else {
            for( unsigned i = 0; i < maxNCars; ++i) {
                unsigned arrivalTime = carsTable[i].getArrivalTime();
                sleep(arrivalTime);
                td[i].travelTime = carsTable[i].getTravelTime();
                td[i].carNo = i;
                td[i].bound = carsTable[i].getBound();
                pthread_create(&cars[i], nullptr, car, (void *) &td[i]);
            }
            delete[] td;
        }
        delete[] cars;
    }

    return 0;
}
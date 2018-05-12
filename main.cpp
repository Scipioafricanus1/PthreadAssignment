#include <iostream>
#include <pthread.h>
#include <utility>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;


static int whittierBound = 0;
static int bearValleyBound = 0;
static int maxNCars;
static int currentNumCars, numCarsWaited = 0;
static string tunnelStatus = "Whittier";
static pthread_mutex_t mutex;
static pthread_cond_t ready = PTHREAD_COND_INITIALIZER;

struct carThread { //container to pass arguments to car pthread.
    unsigned travelTime;
    unsigned carNo;
    string bound;
};

class Cars { //container to hold inputs in vector.
private:
    unsigned arrivalTime;
    string bound;
    unsigned travelTime;
public:
    Cars(unsigned _arrivalTime, string _bound, unsigned _travelTime) {
        arrivalTime = _arrivalTime;
        bound = std::move(_bound);
        travelTime = _travelTime;
    }
    unsigned getArrivalTime() const { return arrivalTime; }
    string getBound() const { return bound; }
    unsigned getTravelTime() const { return arrivalTime; }
};
vector<Cars> carsTable;

void *bearValley(void *arg) //pthread that uses mutex_locks to protect critical areas while in use,
// and sleeps as the car travels through the tunnel
{
    pthread_mutex_lock(&mutex);

    struct carThread *carInfo;
    carInfo = (struct carThread *) arg;
    cout << "Car # " << carInfo->carNo+1 << " going to Bear Valley arrives at tunnel." << endl;
    if (currentNumCars >= maxNCars && tunnelStatus == "Bear Valley") { //only increments when tunnel status is the same.
        numCarsWaited++;
    }
    while(tunnelStatus != "Bear Valley" || currentNumCars >= maxNCars) {
        pthread_cond_wait(&ready, &mutex);
    }

    cout << "Car # " << carInfo->carNo+1 << " going to Bear Valleyenters tunnel." << endl;
    currentNumCars++;

    pthread_mutex_unlock(&mutex);

    sleep(carInfo->travelTime);

    pthread_mutex_lock(&mutex);
    cout << "Car # " << carInfo->carNo+1 << " going to Bear Valley leaves tunnel."<< endl;
    currentNumCars--;
    pthread_cond_broadcast(&ready);
    pthread_mutex_unlock(&mutex);
} // car
void *whittier(void *arg) //whittier car pthread that does same thing as Bear valley pretty much.
{
    pthread_mutex_lock(&mutex);
    struct carThread *carInfo;
    carInfo = (struct carThread *) arg;
    cout << "Car # " << carInfo->carNo+1 << " going to Whittier arrives at tunnel." << endl;
    if (currentNumCars >= maxNCars && tunnelStatus == "Whittier") { //only increments when tunnel status is the same.
        numCarsWaited++;
        //cout <<"BAD LOGIC HERE:"<<endl;
    }
    while(tunnelStatus != "Whittier" || currentNumCars >= maxNCars) {
        pthread_cond_wait(&ready, &mutex);
    }


    cout << "Car # " << carInfo->carNo+1 << " going to Whittier enters tunnel." << endl;
    currentNumCars++;
    pthread_mutex_unlock(&mutex);

    sleep(carInfo->travelTime);

    pthread_mutex_lock(&mutex);
    cout << "Car # " << carInfo->carNo+1 << " going to Whittier leaves tunnel."<< endl;
    currentNumCars--;
    pthread_cond_broadcast(&ready);
    pthread_mutex_unlock(&mutex);
} // car

void *tunnelThread(void *args) {
    //tunnel cycles through 4 choices and sleeps five seconds between each.

    while(true) {
        if(tunnelStatus == "Whittier") {
            printf("Whittier-bound traffic only\n");
            pthread_cond_broadcast(&ready);
            sleep(5);
            tunnelStatus = "No traffic WB";
            printf("No traffic allowed\n");
        } else if (tunnelStatus == "No traffic WB") {
            sleep(5);
            tunnelStatus = "Bear Valley";
            printf("Bear Valley-bound traffic only\n" );
            pthread_cond_broadcast(&ready);
        } else if (tunnelStatus == "Bear Valley") {
            sleep(5);
            tunnelStatus = "No traffic BV";
            printf("Bear Valley-bound traffic only\n");
        } else if(tunnelStatus == "No traffic BV") {
            sleep(5);
            tunnelStatus = "Whittier";
        }
    }

}

string delWhiteSpaces(string &str)
{
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
    str.erase(remove(str.begin(),str.end(), '\n'), str.end());
    str.erase(remove(str.begin(),str.end(),'\t'), str.end());
    return str;
}


int main() {  //reads inputs from user, cycles through table created, making pthreads. Ends program when all car threads are done.
    cin >> maxNCars;
    string line;

    unsigned arrivalTime;
    string bound;
    unsigned travelTime;
    while(cin >> arrivalTime >> bound >> travelTime) {
        carsTable.emplace_back(arrivalTime, bound, travelTime);
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

        td[i].bound = delWhiteSpaces(s);

        sleep(carsTable[i].getArrivalTime());

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

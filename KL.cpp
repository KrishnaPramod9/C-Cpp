#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

// Step 1:
//   V = set of 2n elements
//   {A, B} is an initial partition of V such that
//   A and B are equal-sized subsets of V with no overlap and their union covering all of V.

// Step 2:
//   Compute Dv for all v in V
//   Initialize variables:
//     queue = null;
//     iteration = 1;
//     A' = A;
//     B' = B;

// Step 3:
//   Choose ai from A' and bi from B' which maximizes
//     gi = Dai + Dbi - 2caibi
//   Add pair (ai, bi) to queue
//   Remove ai from A' and bi from B'

// Step 4:
//   If A' and B' are both empty then goto Step 5
//   Else
//     Re-calculate D-values for A' union B'
//     Increment iteration and goto Step 3
//   Endif

// Step 5:
//   Find k to maximize the partial sum of gi from i to k in the queue
//   If G > 0 then
//     Move X = {a1, ..., ak} to B and Y = {b1, ..., bk} to A
//     Goto Step 2
//   Else STOP
//   Endif

class AdjacencyMatrix {
        vector<unsigned short> C;
public:
        int n;
        AdjacencyMatrix(const char* filename) {
                int stage = 0;
                int a, b = 0;
                string line;
                ifstream infile(filename);
                if (infile.is_open())
                {
                        while (infile.good() )
                        {
                                getline(infile,line);
                                switch(stage) {
                                        case 0:
                                                if(sscanf(line.c_str(),"%d",&n) == EOF) {
                                                        cout << "Could not find number of nodes in file." << endl;
                                                        break;
                                                }
                                                C.assign(n*n, 0);
                                                stage = 1;
                                                break;
                                        case 1:
                                                // Ignore the number of nets
                                                stage = 2;
                                                break;
                                        case 2:
                                                if(sscanf(line.c_str(),"%d %d",&a, &b) == EOF) {
                                                        cout << "Ignoring Malformed netlist line: " << line << endl;
                                                        break;
                                                }
                                                // Convert 1-based to 0-based
                                                a--;
                                                b--;
                                                C[min(a, b)*n+max(a, b)]++;
                                                break;
                                        default:
                                                cout << "Invalid parse stage reached!";
                                                break;
                                                break;
                                }
                        }
                        infile.close();
                }
                else cout << "Unable to open file" << endl;
        }

        unsigned short getCost(int i, int j) {
                // Only use the upper-triangular of the matrix
                return C[min(i, j)*n+max(i, j)];
        }

        void setCost(int i, int j, bool b) {
                // Only use the upper-triangular of the matrix
                C[min(i, j)*n+max(i, j)] = b;
        }

        void prettyPrint() {
                cout << "  ";
                for(int i = 0; i < n; i++) {
                        cout << i << " ";
                }
                cout << endl;
                for(int i = 0; i < n; i++) {
                        cout << i << " ";
                        for(int j = 0; j < n; j++) {
                                if(j < i)
                                        cout << "  ";
                                else {
                                        cout << getCost(i, j) << " ";
                                }
                        }
                        cout << endl;
                }
        }
};
typedef vector<bool> LocationVector;
typedef vector<int> GainVector;
class GainQueueItem {
public:
        int indexA;
        int indexB;
        int netGain;
        GainQueueItem(int A, int B, int gain) {
                indexA = A;
                indexB = B;
                netGain = gain;
        }
};
typedef vector<GainQueueItem> GainQueue;

void randomizeVector(LocationVector& V) {
        int n = V.size();
        int n2 = (int)(n/2);
        int rand_index = 0;
        for(int i = 0; i < n2;) {
                rand_index = rand_index() % n;
                if(!V[rand_index]) {
                        V[rand_index] = true;
                        i++;
                }
        }
}

void initializeDValues(GainVector& D, LocationVector& V, AdjacencyMatrix& C) {
        D.clear();
        int n = C.n;
        for(int i = 0; i < n; i++) {
                bool setOfi = V[i];
                int Di = 0;
                for(int j = 0; j < n; j++) {
                        int cij = C.getCost(i, j);
                        if( V[j] == setOfi ) {
                                Di -= cij;
                        }
                        else {
                                Di += cij;
                        }
                }
                D.push_back(Di);
        }
}

int computeCutset(AdjacencyMatrix& C, LocationVector& V) {
        int total = 0;
        int n = C.n;
        int cost = 0;
        for(int i = 0; i < n; i++) {
                for(int j = i; j < n; j++) {
                        cost = C.getCost(i, j);
                        if(V[i] != V[j] and cost != 0)
                                total+= cost;
                }
        }
        return total;
}

void choosePair(GainQueueItem& gi, AdjacencyMatrix& C, LocationVector& V, GainVector& D, vector<bool>& locks) {
        int n = C.n;
        for(int i = 0; i < n; i++) {
                if(locks[i]) continue;
                bool set = V[i];
                for(int j = i + 1; j < n; j++) {
                        if(locks[j] or set == V[j]) continue;
                        int gab = (D[i] + D[j]) - (2 * C.getCost(i, j));
                        //cout << "g" << i << "," << j << ": " << gab << endl;
                        if(gab > gi.netGain) {
                                // Found a better candidate
                                gi.indexA = i;
                                gi.indexB = j;
                                gi.netGain = gab;
                        }
                }
        }
}

void choosePairGreedy(GainQueueItem& gi, AdjacencyMatrix& C, LocationVector& V, GainVector& D, vector<bool>& locks) {
        int n = C.n;
        int bestA = -1;
        int bestB = -1;
        int Da = INT_MIN;
        int Db = INT_MIN;
        int Di = INT_MIN;
        for(int i = 0; i < n; i++) {
                if(locks[i]) continue;
                Di = D[i];
                if(V[i]) {
                        // Set B
                        if(Di > Db) {
                                Db = Di;
                                bestB = i;
                        }
                } else {
                        // Set A
                        if(Di > Da) {
                                Da = Di;
                                bestA = i;
                        }
                }
        }
        gi.indexA = bestA;
        gi.indexB = bestB;
        gi.netGain = (Da + Db) - (2 * C.getCost(bestA, bestB));
}

void recalculateD(GainQueueItem& gi, AdjacencyMatrix& C, LocationVector& V, GainVector& D, vector<bool>& locks) {
        int A = gi.indexA;
        int B = gi.indexB;
        for(int i = 0; i < C.n; i++) {
                if(locks[i]) continue;
                int cia = C.getCost(i, A);
                int cib = C.getCost(i, B);
                if(cia or cib) {
                        if(V[i] == V[A]) {
                                D[i] += 2*cia - 2*cib;
                        } else {
                                D[i] += 2*cib - 2*cia;
                        }
                }
        }
}

int main (int argc, char * const argv[]) {
        // Set the random seed
        srand(720);

    // Parse the input file into the adjacency matrix
        //cout << "Reading file..." << endl;
        char* filename = "../../benchmarks/bench_23.net";
        if(argc > 1)
                filename = argv[1];

        // Declare Variables
        AdjacencyMatrix C = AdjacencyMatrix(filename);
        int n = C.n;
        int n2 = (int)(n/2);
        GainVector D;
        LocationVector V = LocationVector(n, false);
        GainQueue queue;
        vector<bool> locks(n, false);

        //C.prettyPrint();

        // Create a random starting solution
        cout << "Creating a random starting solution..." << endl;
        randomizeVector(V);
        /*
        cout << "V: ";
        for(int i = 0; i < n; i++) {
                char buffer[4]; sprintf(buffer, "%3d", (bool)V[i]);
                cout << buffer << " ";
        }
        cout << endl;
        */

        int maxTotalGain = 1;
        for(int generation = 0; maxTotalGain > 0; generation++) {
                //cout << "Initializing Generation Variables..." << endl;
                maxTotalGain = INT_MIN;
                int maxTotalGainIndex = -1;
                int currentTotalGain = 0;

                // Initialize the D values for all nodes
                //cout << "Initializing D values..." << endl;
                initializeDValues(D, V, C);
                /*
                cout << "D: ";
                for(int i = 0; i < n; i++) {
                        char buffer[4]; sprintf(buffer, "%3d", D[i]);
                        cout << buffer << " ";
                }
                cout << endl;
                 */

                // Step 3 and 4
                for(int iteration = 0; iteration < n2; iteration++) {
                        //cout << "Choosing best-gain pair..." << endl;
                        GainQueueItem gi(0, 0, INT_MIN);
                        choosePairGreedy(gi, C, V, D, locks);
                        cout << "Chose: " << gi.indexA << "," << gi.indexB << ": " << gi.netGain << endl << endl;
                        // Lock the selected nodes for this iteration
                        locks[gi.indexA] = true;
                        locks[gi.indexB] = true;
                        // Push the pair onto the queue and see if it's the best subqueue
                        queue.push_back(gi);
                        currentTotalGain += gi.netGain;
                        if(currentTotalGain > maxTotalGain) {
                                maxTotalGain = currentTotalGain;
                                maxTotalGainIndex = iteration;
                        }
                        // Recalculate D values for still-active nodes
                        if(iteration + 1 < n2) {
                                recalculateD(gi, C, V, D, locks);
                                /*
                                 cout << "D: ";
                                for(int i = 0; i < n; i++) {
                                        char buffer[4]; sprintf(buffer, "%3d", D[i]);
                                        cout << buffer << " ";
                                }
                                cout << endl;
                                */
                        }
                }
                cout << "Queue: " << endl;
                for(int i = 0; i < queue.size(); i++) {
                        GainQueueItem gi = queue[i];
                        cout << i << " (" << gi.indexA << "," << gi.indexB << "): " << gi.netGain << endl;
                }
                cout << "Best Index: " << maxTotalGainIndex << "(" << maxTotalGain << ")" << endl;

                // Apply the optimum subqueue
                //cout << "Applying the subqueue..." << endl;
                if(maxTotalGain > 0) {
                        for(int i = 0; i <= maxTotalGainIndex; i++) {
                                GainQueueItem gi = queue[i];
                                V[gi.indexA].flip();
                                V[gi.indexB].flip();
                        }
                        /*
                        cout << "V: ";
                        for(int i = 0; i < n; i++) {
                                char buffer[4]; sprintf(buffer, "%3d", (bool)V[i]);
                                cout << buffer << " ";
                        }
                        cout << endl;
                        */
                        cout << "Current cutset: " << computeCutset(C, V) << endl << endl;
                }
                // Unlock all nodes
                locks.assign(n, false);
                // Clear the queue
                queue.clear();

        }
        cout << "Final cutset: " << computeCutset(C, V) << endl;
    return 0;
}

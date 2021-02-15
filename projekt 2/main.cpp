#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdio>
using namespace std;

struct Interval{
    int start_idx{};
    int end_idx{};
    int last_used = -1;
};

struct Node{
    vector<int> out;
    int currIntervalKey = 0;    //nodes are in one initial interval as a default
};

//set operations
//void intersection(unordered_set<int>& a, unordered_set<int>& b, unordered_set<int>& result){
//    for (int element : a) {
//        if (b.count(element) == 1) {
//            result.insert(element);
//        }
//    }
//}
//
//
//void difference(unordered_set<int>& a, unordered_set<int>& b, unordered_set<int>& result){
//    for (int element : a) {
//        if (b.count(element) == 0) {
//            result.insert(element);
//        }
//    }
//}


//void updateSetStack(vector<unordered_set<int>>& setStack, Node& u){
//    vector<unordered_set<int>> resultStack;
//    for (unordered_set<int>& X: setStack){
//        unordered_set<int> Y, K;
//        X.size() > u.out.size()? intersection(u.out, X, Y) : intersection(X, u.out, Y);
//        difference(X, Y, K);
//        if (!K.empty()){
//            resultStack.push_back(K);
//        }
//        if (!Y.empty()){
//            resultStack.push_back(Y);
//        }
//    }
//    setStack = resultStack;
//}
//
//void lexBFS(vector<int>& ordering, vector<Node>& graph){
//    int n = graph.size();
//
//    unordered_set<int> initial_set;
//    for (int i = 0; i < n; i++){
//        initial_set.insert(i);
//    }
//    vector<unordered_set<int>> setStack;
//    setStack.push_back(initial_set);
//
//    for (int i = 0; i < n; i++) {
//        int u_ind = *setStack.back().begin();
//        setStack.back().erase(u_ind);
//        Node u = graph[u_ind];
//
//        ordering.push_back(u.index);
//        updateSetStack(setStack, u);
//    }
//}


//int largestClique(vector<Node>& graph){
//    int n = graph.size();
//    vector<int> peo;
//    lexBFS(peo, graph);    //graf jest przekątniowy => porządek leksykograficzny = PEO
//
//    int maxClique = 2;
//    for (int i = n-1; i >= 0; i--){
//        if (i < maxClique){
//            break;
//        }
//        int u_ind = peo[i];
//        Node u = graph[u_ind];
//
//        int currClique = 1; //wliczamy obecny wierzchołek
//        for (int j = 0; j < i; j++){
//            if (u.out.count(peo[j]) == 1){
//                currClique++;
//            }
//        }
//        if (currClique > maxClique){
//            maxClique = currClique;
//        }
//    }
//    return maxClique;
//}

inline void sWAP(vector<int>& order, vector<int>& where, int i, int j){     //i, j - current positions of elements to swap
    swap(order[i], order[j]);
    swap(where[order[i]], where[order[j]]);
}

inline void addToIntervalMap(unordered_map<int, Interval>& interval_map, int start, int end, int& maxIntervalKey){
    Interval new_interval;
    new_interval.start_idx = start;
    new_interval.end_idx = end;

    maxIntervalKey++;
    interval_map[maxIntervalKey] = new_interval;
}

inline void removeVertexFromInterval(unordered_map<int, Interval>& interval_map, int key){
    interval_map[key].end_idx--;
    if (interval_map[key].start_idx > interval_map[key].end_idx)
        interval_map.erase(key);
}

//we find successor of the interval based on the key in the node next to the last node in the interval
int findNextIntervalKey(Interval& old_interval, vector<int>& order, vector<Node>& graph){
    int vertex_in_new_interval = order[old_interval.end_idx + 1];
    return graph[vertex_in_new_interval].currIntervalKey;
}

int solve(vector<Node>& graph){
    int n = graph.size();
    int max_clique = 2;

    vector<int> order(n);       //order[i] - after the execution of the function, this will be peo of the given chordal graph
    vector<int> where(n);       //where[i] - index of vertex number i (points to current position) in order vertex
    for (int i = 0; i < n; i++){
        order[i] = i;
        where[i] = i;
    }

    int max_interval_key = -1;
    unordered_map<int, Interval> interval_map;
    addToIntervalMap(interval_map, 0, n-1, max_interval_key);

    for (int i = n-1; i >= 0; i--){
        //taking the right vertex
        int u_idx = order[i];
        removeVertexFromInterval(interval_map, graph[u_idx].currIntervalKey);

        int curr_clique = 1;

        for (int v: graph[u_idx].out){ //for each vertex that is a neighbour of the current node
            int v_position = where[v];  //position of v in graph ordering
            if (v_position < i){  //when neighbour is on the left of the current node
                Interval old_interval = interval_map[graph[v].currIntervalKey];
                sWAP(order, where, v_position, old_interval.end_idx);       //swapping of two elements
                v_position = where[v];
                removeVertexFromInterval(interval_map, graph[v].currIntervalKey);  //removing v from old interval

                if (old_interval.last_used != i){    //if the old_interval wasn't already used
                    interval_map[graph[v].currIntervalKey].last_used = i;  //we note that the old interval was already used

                    addToIntervalMap(interval_map, v_position, v_position, max_interval_key);
                    graph[v].currIntervalKey = max_interval_key;
                }
                else {
                    int new_interval_key = findNextIntervalKey(old_interval, order, graph);
                    graph[v].currIntervalKey = new_interval_key;
                    interval_map[new_interval_key].start_idx--;
                }
            }
            else{   //if vertex is on right position, it must be in the clique
                curr_clique++;
            }
        }

        if (curr_clique > max_clique)
            max_clique = curr_clique;
    }
    return max(max_clique-1, 2);
}

int stdioLikeInput() {
    int result = 0;

    char input = getc_unlocked(stdin);
    while (input >= '0' && input <= '9') {
        result = 10 * result + (input - (int)'0');
        input = getc_unlocked(stdin);
//        if (input == '\r')
//            getc_unlocked(stdin);
    }

    return result;
}


int main() {
    int Z = stdioLikeInput();
    while (Z--){
        int N = stdioLikeInput();
        int M = stdioLikeInput();

        vector<Node> graph(N);
        for (int i = 0; i < M; i++){
            int u = stdioLikeInput();
            int v = stdioLikeInput();

            graph[u-1].out.push_back(v-1);
            graph[v-1].out.push_back(u-1);
        }
        cout << solve(graph) << endl;

    }
    return 0;
}
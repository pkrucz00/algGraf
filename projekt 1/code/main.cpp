#include <iostream>
#include <deque>
#include <vector>
#include <algorithm>

using namespace std;

struct Edge{
    int to;         //index of adjacent vertex
    int capacity;
    int cost;
};

struct Game{
    int wins;
    int loses;
    int bribe;
};

void addEdge(vector<vector<Edge>>& graph, int from, int to, int capacity, int cost){
    Edge edge {
        .to = to,
        .capacity = capacity,
        .cost = cost
    };
    graph[from].push_back(edge);
}

void initGraph(vector<vector<Edge>>& graph, vector<Game>& games, int n, int B){
    int nOfVertices = n+3;   //the are n+3 vertexes. First n is for players,
    int source = n;           // n+1st is for source,
    int trafJam = n+1;        // (n+2)nd - "traffic jam" vertex
    int sink = n+2;           // and n+3rd for sink
    graph.resize(nOfVertices);

    for (int i = 0; i < n; i++){
        addEdge(graph, source, i, 0, 0);
        addEdge(graph, i, source, 0, 0);
    }

    for (auto& game: games){
        if (game.bribe <= B && game.wins != 0){
            addEdge(graph, game.wins, game.loses, 0, game.bribe);
            addEdge(graph, game.loses, game.wins, 0, -game.bribe);
        }
    }

    addEdge(graph, 0, sink, 0, 0);
    addEdge(graph, sink, 0, 0, 0);

    for (int i = 1; i < n; i++){
        addEdge(graph, i, trafJam, 0, 0);
        addEdge(graph, trafJam, i, 0, 0);
    }
    addEdge(graph, trafJam, sink, 0, 0);
    addEdge(graph, sink, trafJam, 0, 0);
}

Edge& findEdge(vector<vector<Edge>>& graph, int from, int to){
    return *find_if(graph[from].begin(), graph[from].end(), [to](Edge &edge) { return edge.to == to; });
}


int initFlow(vector<vector<Edge>>& graph,vector<Game>& games, vector<int>& wonGames, int n, int threshold, int B){
    int kingsFlow = min(threshold, wonGames[0]);
    int accFlow = kingsFlow;


    //players' capacity is always 1 for the winner, 0 for the loser
    for (auto& game:games){
        if (game.bribe <= B && game.wins != 0){
            Edge& edgeFromWinner = findEdge(graph, game.wins, game.loses);
            Edge& edgeFromLoser = findEdge(graph, game.loses, game.wins);
            edgeFromWinner.capacity = 1;
            edgeFromLoser.capacity = 0;
        }
    }

    //kings flow given by threshold
    Edge& edgeFromSourceToKing = findEdge(graph, n, 0);
    Edge& edgeFromKingToSource = findEdge(graph, 0, n);
    Edge& edgeFromKingToTarget = findEdge(graph, 0, n+2);
    Edge& edgeFromTargetToKing = findEdge(graph, n+2, 0);

    edgeFromSourceToKing.capacity = wonGames[0] - kingsFlow;
    edgeFromKingToSource.capacity = kingsFlow;
    edgeFromKingToTarget.capacity = threshold - kingsFlow;
    edgeFromTargetToKing.capacity = kingsFlow;


    int trafficJamCapacity = n*(n-1)/2 - threshold;
    for (int i = 1; i < n; i++){
        int playersFlow = min(threshold, min(wonGames[i], trafficJamCapacity));
        trafficJamCapacity -= playersFlow;
        accFlow += playersFlow;

        Edge& fromSourceToPlayer = findEdge(graph, n, i);
        Edge& fromPlayerToSource = findEdge(graph, i, n);
        fromSourceToPlayer.capacity = wonGames[i] - playersFlow;
        fromPlayerToSource.capacity = playersFlow;

        Edge& fromPlayerToTrafJam = findEdge(graph, i, n+1);
        Edge& fromTrafJamToPlayer = findEdge(graph, n+1, i);
        fromPlayerToTrafJam.capacity = threshold - playersFlow;
        fromTrafJamToPlayer.capacity = playersFlow;
    }

    Edge& fromTrafJamToSink = findEdge(graph, n+1, n+2);
    Edge& fromSinkToTrafJam = findEdge(graph, n+2, n+1);
    fromTrafJamToSink.capacity = trafficJamCapacity;
    fromSinkToTrafJam.capacity = n*(n-1)/2 - threshold - trafficJamCapacity;

    return accFlow;
}

bool SPFA(vector<vector<Edge>>& graph, vector<Game>& games, vector<int>& wonGames, int n, int threshold, int B){
    deque<int> queue;
    vector<bool> inQueue(n+3, false);

    int flow = initFlow(graph, games, wonGames, n, threshold, B);
    int cost = 0;
    bool searchPath = true;

    while (searchPath){
        pair<int,int> initData(INT32_MAX, -1);  //first - distance, second - parent
        vector<pair<int,int>> vertices(n+3, initData);
        vertices[n].first = 0;   //setting distance of source to 0

        queue.push_back(n);
        inQueue[n] = true;

        while(!queue.empty()){
            int u = queue.front();
            queue.pop_front();
            inQueue[u] = false;

            for (auto& edge: graph[u]){
                int v = edge.to;
                if (edge.capacity > 0 && vertices[v].first > vertices[u].first + edge.cost){
                    vertices[v].first = vertices[u].first + edge.cost;
                    vertices[v].second = u;
                    if (!inQueue[v]){
                        queue.push_back(v);
                        inQueue[v] = true;
                    }
                }
            }
        }


        if (vertices[n+2].second == -1)   //if sink has no parent there is no path
            searchPath = false;
        else{
            int curr = n+2;
            int prev = vertices[n+2].second;
            int minCapacity = INT32_MAX;

            while (prev != -1) {
                Edge& edge = findEdge(graph, prev, curr);

                if (edge.capacity < minCapacity) {
                    minCapacity = edge.capacity;
                }
                curr = vertices[curr].second;
                prev = vertices[prev].second;
            }

            flow += minCapacity;

            curr = n+2;
            prev = vertices[n+2].second;

            while (prev != -1) {
                Edge& edgeFromPrevToCurr = findEdge(graph, prev, curr);
                Edge& edgeFromCurrToPrev = findEdge(graph, curr, prev);
                edgeFromPrevToCurr.capacity -= minCapacity;
                edgeFromCurrToPrev.capacity += minCapacity;
                cost += edgeFromPrevToCurr.cost * minCapacity;

                curr = vertices[curr].second;
                prev = vertices[prev].second;
            }
        }
    }
    return (flow == n*(n-1)/2 && cost <= B);
}


bool canKingWin(vector<Game>& games, vector<int>& wonGames, int n, int B){
    int minWins = n/2;    //smallest number of games a player must win in order to win a round robin competition
    int kingsWins = wonGames[0];
    int threshold = max(minWins, kingsWins);

    vector<vector<Edge>> graph;
    initGraph(graph, games, n, B);

    for (int i = threshold; i < n; i++){
        if (SPFA(graph,games, wonGames, n, i, B)){
            return true;
        }
    }
    return false;
}

int main() {
    int Z;
    cin >> Z;
    while(Z--){
        int B, n;
        cin >> B >> n;

        if (n==1){
            cout << "TAK" << endl;
            continue;
        }

        vector<Game> games;
        vector<int> wonGames(n);

        for (int i = 0; i < n*(n-1)/2; i++){
            int x, y, w, b;
            cin >> x >> y >> w >> b;

            Game game{};
            if (x == w){
                game.wins = x;
                game.loses = y;
            } else {
                game.wins = y;
                game.loses = x;
            }
            game.bribe = b;
            games.push_back(game);

            wonGames[w]++;
        }

        if (canKingWin(games, wonGames, n, B)){
            cout << "TAK" << endl;
        } else
            cout << "NIE" << endl;
    }
    return 0;


}

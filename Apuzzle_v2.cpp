#include <iostream>
#include <vector>
#include <stack>
#include <queue> 
#include <map>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct node{
  int state[3][3]; // board
  double h; // heuristic value
  int g = 0; // cost or depth. All movements cost 1
  string action = ""; // string describing the action : up, down, left, right
  vector<node> parent; // stores info on the parent node for "traversing" back to root node. I dont think a vector is necessary.
};

/*
compares the f(n) values of 2 states for priority queue ordering where the smallest f(n) is given priority
smallest f(n) is next to pop off queue. 
f(n) = g(n) + h(n)
*/
struct CompareStates {
    bool operator()(node const& state1, node const& state2){
        return (state1.g + state1.h) >= (state2.g + state2.h);
    }
};

/*
Angelica has 2 As that can be swapped and still spell Angelica, meaning there are essentially 2 goal states.
Did not use letters because I thought distinguishing between A and A in heuristic calculation was harder than finding it twice.
*/
int goalState1[3][3] = {{1,2,3}, {4,5,6}, {7,8,0}};
int goalState2[3][3] = {{8,2,3}, {4,5,6}, {7,1,0}};

/*
maps allow for easy conversion from letter to numbers and vise versa 
*/
map<int, string> num_to_char{{0, "-"}, {1, "A"}, {2, "N"}, {3, "G"}, {4, "E"}, {5, "L"}, {6, "I"}, {7, "C"}, {8, "A"}};
map<string, int> char_to_num{{"-", 0}, {"A", 1}, {"N", 2}, {"G", 3}, {"E", 4}, {"L", 5}, {"I", 6}, {"C", 7}, {"a", 8}};

vector<node> seen; // vector of all the states already encountered

// check if state is goal state 1
bool goalTest1(node test){
	for(int i = 0; i < 3; ++i){
	    for(int j = 0; j < 3; ++j){
	      if(test.state[i][j] != goalState1[i][j]){
	        return false;
	      }
	    }
	}
	return true;
}

// check if state is goal state 2
bool goalTest2(node test){
	for(int i = 0; i < 3; ++i){
	    for(int j = 0; j < 3; ++j){
	      if(test.state[i][j] != goalState2[i][j]){
	        return false;
	      }
	    }
	}
	return true;
}

/*
calculates the heuristic value.
1 = UC : h(n) = 0 
2 = Misplaced : h(n) = number of tiles that are not in the correct place
3 - Manhattan distance : h(n) =  Manhattan distance between each misplaced tile and where it should be
returns the smallest heuristic of the 2 goalstates
*/
int heuristic(node test, int algo){
    // 1 = UC : h(n) = 0 
	if(algo == 1){;
		//h = 0
		return 0;
	}
    // 2 = Misplaced : h(n) = number of tiles that are not in the correct place
	else if(algo == 2){
		int numwrong1 = 0; // number of misplaced tiles using goalState1
        int numwrong2 = 0; // number of misplaced tiles using goalState2
		for(int i = 0; i < 3; i++){
			for(int j = 0; j < 3; j++){
				if(test.state[i][j] == 0){  // if the tile = 0 ignore it. technically it is not an actual tile
					continue;
				}
                // if a mismatch is detected, increment respective counter
				if(test.state[i][j] != goalState1[i][j]){   
					numwrong1++;
				}
                if(test.state[i][j] != goalState2[i][j]){
					numwrong2++;
				}
			}
		}
        // return smallest counter
        if(numwrong1 < numwrong2){
            return numwrong1;
        }
		return numwrong2;
	}
	else if(algo == 3){
        /*
        3 - Manhattan distance : h(n) =  Manhattan distance between each misplaced tile and where it should be
		h = total distance every wrong tile needs to move
		ex : “3”, “8” and “1” tiles are misplaced, by 2, 3, and 3 squares 
		so h = 2+3+3=8.
        */
		int sumDistance1 = 0;   // total Manhattan distance using goalState1 as reference
        int sumDistance2 = 0;   // total Manhattan distance using goalState2 as reference
		int xindex;             // x coord of tile you want to find distance on
		int yindex;             // y coord of tile you want to find distance on
		int goalx_1;            // x coord of target tile in goalState1 
		int goaly_1;            // y coord of target tile in goalState1 
        int goalx_2;            // x coord of target tile in goalState2
		int goaly_2;            // y coord of target tile in goalState2

        // iterated through each numbered tile
		for(int tile = 1; tile < 9; tile++){
			xindex = -1;
			yindex = -1;
			goalx_1 = -1;
			goaly_1 = -1;
            goalx_2 = -1;
			goaly_2 = -1;

            // iterate through the whole board
			for(int y = 0; y < 3; y++){
				for(int x = 0; x < 3; x++){

                    // if the coords on the current board is the tile we are comparing with the goal state, store it
					if(test.state[y][x] == tile){
						xindex = x;
						yindex = y;
					}

                    // if the coords on the goalState1 board is the target tile, store it
					if(goalState1[y][x] == tile){
						goalx_1 = x;
						goaly_1 = y;
					}

                    // if the coords on the goalState2 board is the target tile, store it
                    if(goalState2[y][x] == tile){
						goalx_2 = x;
						goaly_2 = y;
					}
				}
			}

            // sum up all distances
			sumDistance1 = sumDistance1 + abs(xindex - goalx_1) + abs(yindex - goaly_1);
            sumDistance2 = sumDistance2 + abs(xindex - goalx_2) + abs(yindex - goaly_2);
		}
        // return smallest distance
        if(sumDistance1 < sumDistance2){
            return sumDistance1;
        }
		return sumDistance2;
	}
    // in case of invalid input, exit.
	else{
		cout << "Invalid input" << endl;
		exit(1);
	}
}

/*
Given a node and what move to make, create a new node representing the board after the move was made
blankX and blankY are the coords for the 0 aka blank tile
based on move, swap the tiles fill in the rest of the info for the node and return it
*/
node move(node original, int algo, int blankX, int blankY, string move ){
	node copy;

    // copy the board of the original to the copy node
	for(int i = 0; i < 3; ++i){
	    for(int j = 0; j < 3; ++j){
	      copy.state[i][j] = original.state[i][j];
	    }
	}

    // swap the tiles
	if(move == "left"){
		copy.state[blankY][blankX] = copy.state[blankY][blankX + 1];
		copy.state[blankY][blankX + 1] = 0;
	}
	else if(move == "right"){
		copy.state[blankY][blankX] = copy.state[blankY][blankX - 1];
		copy.state[blankY][blankX - 1] = 0;
	}
	else if(move == "up"){
		copy.state[blankY][blankX] = copy.state[blankY+1][blankX];
		copy.state[blankY+1][blankX] = 0;
	}
	else if(move == "down"){
		copy.state[blankY][blankX] = copy.state[blankY-1][blankX];
		copy.state[blankY-1][blankX] = 0;
	}
	else{
		cout << "impossible move " << move << endl;
		exit(1); 
	}

	copy.parent.push_back(original);    // store the original so we know where this new node came from
	copy.action = move;                 // store the move name so we can output the actions needed to solve
	copy.g = original.g + 1;            // copy's g is 1 greate than original's g because it one step lower in the tree
	copy.h = heuristic(copy, algo);     // find the heuristics of the new node and store it so we can push it to the priority queue
	return copy;
}


/*
check to see if the node has been visited before. If so, return false so that it won't be expanded again.
I tried using a hash table but it wasnt working out. the current method works fine albeit slow, but you cant tell for shallow solutions.
*/
bool visited(node &test){
	// iterate through the seen vector
	bool same;
	for(int i = 0; i < seen.size(); i++){
		same = true;
		node curr = seen.at(i);
		//compare the states. If even 1 tile is different, we can move on to the next state
		for(int y = 0; y < 3; y++){
			for(int x = 0; x < 3; x++){
				if(test.state[y][x] != curr.state[y][x]){
					// did not record data for report using the check below because I caught it after recording all the data
					if(test.state[y][x] == 1 || test.state[y][x] == 8){
						if (curr.state[y][x] == 1 || curr.state[y][x] == 8){
							continue;
						}
					}
					same = false;
					break;
				}
			}
			if(!same){
				break;
			}
		}
		if(same){
			/*
			Sometimes, because of the heuristic, a duplicate state with higher g(n) is stored in seen vector first.
			This means if a duplicate state is found later but with a smaller g(n), it will not be expanded.
			This causes the algorithm to continue expanding the state with higher g(n), making the returned solution sub-optimal.
			For now I used a naive solution to fix this issue. If the state g(n) is < the g(n) of the state stored in seen, return false.
			This causes the duplicate state to be inserted into the frontier priority queue and expanded first, preventing suboptimal expansions
			from continuing.
			Cons : Not very space efficient because more dup states are stored in the priority queue and seen vector stores dup state as well.
			*/
			if(test.g < curr.g){ 
                return false;
            }
			return true;
		}
	}
	return false;
}

// print the state including g(n) and h(n)
void printState(node state){
	cout << "The best state to expand with g(n) = " << state.g << " and h(n) = " << state.h << endl;
	for(int i = 0; i < 3; ++i){
	    for(int j = 0; j < 3; ++j){
	      cout << num_to_char.find(state.state[i][j])->second << " ";
	    }
    cout << endl;
  }
}

node search(node start, int algo){
    auto begin = high_resolution_clock::now();	// record current time
	int expanded = 0;
	int maxSize = 0;

	/*
	priority queue to make getting next state easier
	automatically orders states so that states with the lowest f(n) are prioritized
	*/
    priority_queue<node, vector<node>, CompareStates> frontier;
	
	// push start to be first in the queue
	start.h = heuristic(start, algo);
	frontier.push(start);

	node current;

	/*
	while loop continues until queue is empty in which case there is no solution
	update max queue size
	get the next state
	check if it is goal state
	expand node
	*/
	while(!frontier.empty()){
		// keep track of max queue size
		if(frontier.size() > maxSize){
			maxSize = frontier.size();
		}

		// set current to next node of frontier and remove it from the queue
		current = frontier.top();
        frontier.pop();

		// if current is goal state then return success; start printing results
		if(goalTest1(current) || goalTest2(current)){
            auto stop = high_resolution_clock::now();	// record current time
            auto duration = duration_cast<microseconds>(stop - begin);	// get the difference
			cout << "Goal!!!" << endl;
			// how many nodes expanded
			cout << "To solve this problem the search algorithm expanded a total of "
				<< expanded << " nodes" << endl;
			// max num of nodes in queue at once
			cout << "The maximum number of nodes in the queue at any one time: " << maxSize << endl;
			// what was depth of the goal (how many moves)
			cout << "The depth of the goal node was " << current.g << endl;
			// how long did it take to find the solution
            cout << "Time : " << duration.count() << " microseconds" << endl;
			// print solution
			// traverse back to root node storing the "action" in a stack 
			stack<string> sequence;
			node backtrack = current;
			while(backtrack.g != 0){
				sequence.push(backtrack.action);
			    backtrack = backtrack.parent.front();
			}
			// print out the stored actions
			while(!sequence.empty()){
			    cout << sequence.top() << endl;
			    sequence.pop();
			}
			return current;
		}

		// expand current node (left, right, up, down) 
		expanded++;	// increment counter

		// find the coords of 0
		int zerox = 0;
		int zeroy = 0;
		for(int i = 0; i < 3; ++i){
			for(int j = 0; j < 3; ++j){
		    	if(current.state[i][j] == 0){
		        	zeroy = i;
		        	zerox = j;
		      	}
		    }
		}
		node newNode;
		switch(zeroy){
			case 0:
				/*
				handles the case where 0 is on the first row
				tile can be moved up (0 cannt be moved up if it is on the top row)
				since 0 is on the first row, there is no tile above it to move down
				*/
				newNode = move(current, algo, zerox, zeroy, "up");
				if(!visited(newNode)){
				    frontier.push(newNode);
				    seen.push_back(newNode);
				}

				// as long as 0 is not on the last column, there is a tile right of it that can move left
				if(zerox < 2){ 
					newNode = move(current, algo, zerox, zeroy, "left");
					if(!visited(newNode)){
    				    frontier.push(newNode);
    				    seen.push_back(newNode);
    				}
				}

				// as long as 0 is not on the first column, there is a tile left of it that can move right
				if(zerox > 0){ 
					newNode = move(current, algo, zerox, zeroy, "right");
					if(!visited(newNode)){
    				    frontier.push(newNode);
    				    seen.push_back(newNode);
    				}
				}
				break;
			case 1:
				/*
				handles the case where 0 is on the second row
				tile can be moved up. if 0 is on the 2nd row, there is a 3rd row where a tile can be moved up
				since 0 is on the 2nd row, there is a tile above it on the 1st row that can move down
				*/
				newNode = move(current, algo, zerox, zeroy, "up");
				if(!visited(newNode)){
				    frontier.push(newNode);
				    seen.push_back(newNode);
				}

				// since 0 is on the 2nd row, there is a tile above it on the 1st row that can move down 
				newNode = move(current, algo, zerox, zeroy, "down");
				if(!visited(newNode)){
				    frontier.push(newNode);
				    seen.push_back(newNode);
				}

				// as long as 0 is not on the last column, there is a tile right of it that can move left
				if(zerox < 2){ 
					newNode = move(current, algo, zerox, zeroy, "left");
					if(!visited(newNode)){
    				    frontier.push(newNode);
    				    seen.push_back(newNode);
    				}
				}

				// as long as 0 is not on the first column, there is a tile left of it that can move right
				if(zerox > 0){ 
					newNode = move(current, algo, zerox, zeroy, "right");
					if(!visited(newNode)){
    				    frontier.push(newNode);
    				    seen.push_back(newNode);
    				}
				}
				break;
			case 2:
				/*
				handles the case where 0 is on the last row
				tile cant be moved up. if 0 is on the last row, there is no tile below it that can be moved up
				since 0 is on the last row, there is a tile above it to move down
				*/
				newNode = move(current, algo, zerox, zeroy, "down");
				if(!visited(newNode)){
				    frontier.push(newNode);
				    seen.push_back(newNode);
				}

				// as long as 0 is not on the last column, there is a tile right of it that can move left
				if(zerox < 2){ 
					newNode = move(current, algo, zerox, zeroy, "left");
					if(!visited(newNode)){
    				    frontier.push(newNode);
    				    seen.push_back(newNode);
    				}
				}

				// as long as 0 is not on the first column, there is a tile left of it that can move right
				if(zerox > 0){ 
					newNode = move(current, algo, zerox, zeroy, "right");
					if(!visited(newNode)){
    				    frontier.push(newNode);
    				    seen.push_back(newNode);
    				}
				}
				break;
			default:
				cout << "Something went wrong. Zero was found at " << zerox << ", " << zeroy << endl;
				exit(1);
		}
		printState(current);
	}
	// no solution
	cout << "No solution. Impossible to solve." << endl;
	exit(1);
}

int main(){
	// get user input on puzzle set up
	cout << "Welcome to 862179510 Angelica puzzle solver." << endl
		<< "Type \"1\" to use a default puzzle (test cases), or \"2\" to enter your own puzzle.";
	int option;
	cin >> option;
	node start;
	if(option == 1){
		cout << "Enter the test case number." << endl;
		cout << "1: Depth 0 \n2: Depth 2 \n3: Depth 4 \n4: Depth 8 \n5: Depth 12 \n6: Depth 16 \n7: Depth 20 \n8: Depth 24" << endl;
		int testcase;
		cin >> testcase;
		if(testcase == 1){
			start.state[0][0] = 1;
			start.state[0][1] = 2;
			start.state[0][2] = 3;
			start.state[1][0] = 4;
			start.state[1][1] = 5;
			start.state[1][2] = 6;
			start.state[2][0] = 7;
			start.state[2][1] = 8;
			start.state[2][2] = 0;
		}
		else if(testcase == 2){
			start.state[0][0] = 1;
			start.state[0][1] = 2;
			start.state[0][2] = 3;
			start.state[1][0] = 4;
			start.state[1][1] = 5;
			start.state[1][2] = 6;
			start.state[2][0] = 0;
			start.state[2][1] = 7;
			start.state[2][2] = 8;
		}
		else if(testcase == 3){
			start.state[0][0] = 1;
			start.state[0][1] = 2;
			start.state[0][2] = 3;
			start.state[1][0] = 5;
			start.state[1][1] = 0;
			start.state[1][2] = 6;
			start.state[2][0] = 4;
			start.state[2][1] = 7;
			start.state[2][2] = 8;
		}
		else if(testcase == 4){
			start.state[0][0] = 1;
			start.state[0][1] = 3;
			start.state[0][2] = 6;
			start.state[1][0] = 5;
			start.state[1][1] = 0;
			start.state[1][2] = 2;
			start.state[2][0] = 4;
			start.state[2][1] = 7;
			start.state[2][2] = 8;
		}
		else if(testcase == 5){
			start.state[0][0] = 1;
			start.state[0][1] = 3;
			start.state[0][2] = 6;
			start.state[1][0] = 5;
			start.state[1][1] = 0;
			start.state[1][2] = 7;
			start.state[2][0] = 4;
			start.state[2][1] = 8;
			start.state[2][2] = 2;
		}
		else if(testcase == 6){
			start.state[0][0] = 1;
			start.state[0][1] = 6;
			start.state[0][2] = 7;
			start.state[1][0] = 5;
			start.state[1][1] = 0;
			start.state[1][2] = 3;
			start.state[2][0] = 4;
			start.state[2][1] = 8;
			start.state[2][2] = 2;
		}
		else if(testcase == 7){
			start.state[0][0] = 7;
			start.state[0][1] = 1;
			start.state[0][2] = 2;
			start.state[1][0] = 4;
			start.state[1][1] = 8;
			start.state[1][2] = 5;
			start.state[2][0] = 6;
			start.state[2][1] = 3;
			start.state[2][2] = 0;
		}
		else if(testcase == 8){
			start.state[0][0] = 0;
			start.state[0][1] = 7;
			start.state[0][2] = 2;
			start.state[1][0] = 4;
			start.state[1][1] = 6;
			start.state[1][2] = 1;
			start.state[2][0] = 3;
			start.state[2][1] = 5;
			start.state[2][2] = 8;
		}
		else{
			cout << "Invalid input" << endl;
			exit(1);
		}
	}
	else if(option == 2){
		string right, center, left;	// temp variable to store user inputs
		cout << "Enter your puzzle using capital letters only and a \"-\" to represent the blank. Please only enter valid Angelica-puzzles." << endl;
		cout << "Enter the puzzle demilimiting the letters with a space. Press the ENTER key only when finished." << endl;
		bool existsA = false;
		cout << "Enter the first row, use space or tabs between numbers : ";
		cin >> left;
		if(left == "A"){existsA = true;}
		cin >> center;
		if(center == "A"){
			if(existsA == true){
				center = "a";
			}
			else{existsA = true;}
		}
		cin >> right;
		if(right == "A"){
			if(existsA == true){
				right = "a";
			}
			else{existsA = true;}
		}
		// conver strings to ints using char_to_num
		start.state[0][0] = char_to_num.find(left)->second;
    	start.state[0][1] = char_to_num.find(center)->second;
    	start.state[0][2] = char_to_num.find(right)->second;
		
		// repeat for 2nd and 3rd row
    	cout << "Enter the second row, use space or tabs between numbers : ";
		cin >> left;
		if(left == "A"){
			if(existsA == true){
				left = "a";
			}
			else{existsA = true;}
		}
		cin >> center;
		if(center == "A"){
			if(existsA == true){
				center = "a";
			}
			else{existsA = true;}
		}
		cin >> right;
		if(right == "A"){
			if(existsA == true){
				right = "a";
			}
			else{existsA = true;}
		}
		start.state[1][0] = char_to_num.find(left)->second;
    	start.state[1][1] = char_to_num.find(center)->second;
    	start.state[1][2] = char_to_num.find(right)->second;
    	cout << "Enter the third row, use space or tabs between numbers : ";
		cin >> left;
		if(left == "A"){
			if(existsA == true){
				left = "a";
			}
			else{existsA = true;}
		}
		cin >> center;
		if(center == "A"){
			if(existsA == true){
				center = "a";
			}
			else{existsA = true;}
		}
		cin >> right;
		if(right == "A"){
			if(existsA == true){
				right = "a";
			}
			else{existsA = true;}
		}
		start.state[2][0] = char_to_num.find(left)->second;
    	start.state[2][1] = char_to_num.find(center)->second;
    	start.state[2][2] = char_to_num.find(right)->second;
	}
	else{
		cout << "Invalid input" << endl;
		exit(1);
	}
	// get user input for heuristic 
	option = 0;
	cout << "Enter your choice of algorithm (enter number)"<< endl
		<< "1. Uniform Cost Search" << endl
		<< "2. A* with the Misplaced Tile heuristic" << endl
		<< "3. A* with the Manhattan Distance heuristic" << endl;
	cin >> option;
	if(option == 1){;
		//run algo with h = 0
		search(start, 1);
	}
	else if(option == 2){
		//run algo with h = # tiles not correct
		search(start, 2);
	}
	else if(option == 3){
		search(start, 3);
	}
	else{
		cout << "Invalid input" << endl;
		exit(1);
	}

	return 0;
}
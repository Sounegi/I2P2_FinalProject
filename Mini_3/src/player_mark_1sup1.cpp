#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <set>

#define max(a, b) a > b ? a : b;
#define min(a, b) a < b ? a : b;

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int player;
int opposite_player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
//int player_disc;
//int opposite_disc;
std::vector<Point> next_valid_spots;
std::vector<int> spots_value;
const std::array<Point, 8> directions{{
    Point(-1, -1), Point(-1, 0), Point(-1, 1),
    Point(0, -1), /*{0, 0}, */Point(0, 1),
    Point(1, -1), Point(1, 0), Point(1, 1)
}};

using Board = std::array<std::array<int, SIZE>, SIZE>;

bool is_spot_on_board(Point p) {
    return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
}
void set_disc(Board b, Point p, int disc){
    b[p.x][p.y] = disc;
}
int get_disc(Board b, Point p){
    return b[p.x][p.y];
}
bool is_disc_at(Board b, Point p, int disc){
    if(!is_spot_on_board(p))
        return false;
    if(get_disc(b, p) != disc)
        return false;
    return true;
}
void flip_disc(Board b, Point center, int player){
    int opposite =  3 - player;
    for(Point dir: directions){
        Point p = center + dir;
        if(!is_disc_at(b, p, opposite))
            continue;
        std::vector<Point> discs({p});
        p = p + dir;
        while(is_spot_on_board(p) && get_disc(b, p) != 0){
            if(is_disc_at(b, p, player)){
                for(Point s: discs){
                    set_disc(b, s, player);
                }
                break;
                //we count later
            }
            discs.push_back(p);
            p = p + dir;
        }
    }
}
bool is_spot_valid(Board b, Point center, int player){
    int opposite = 3 - player;
    if(get_disc(b, center) != 0)
        return false;
    for(Point dir: directions){
        Point p = center + dir;
        if(!is_disc_at(b, p, opposite))
            continue;
        p = p + dir;
        while(is_spot_on_board(p) && get_disc(b, p) != 0){
            if(is_disc_at(b, p, player))
                return true;
            p = p + dir;
        }
    }
    return false;
}
std::vector<Point> get_valid_spots(Board b, int player) {
    std::vector<Point> valid_spots;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            Point p = Point(i, j);
            if (b[i][j] != 0)
                continue;
            if (is_spot_valid(b, p, player))
                valid_spots.push_back(p);
        }
    }
    return valid_spots;
}
int count_value(Board b){
    int player_disc = 0;
    int opposite_disc = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if(b[i][j] == player) player_disc++;
            if(b[i][j] == opposite_player) opposite_disc++;
        }
    }
    return player_disc - opposite_disc;
}

Board predict_board(Board cur_board, Point move, int player){
    //no need to check valid since choose from valid spot
    Board new_b = cur_board;
    //int opposite = 3 - player;
    set_disc(new_b, move, player);
    flip_disc(new_b, move, player);
    return new_b;
}

int minimax(Board board, int depth, int cur_player){
    int value;
    std::vector<Point> board_valid_spots = get_valid_spots(board, player);
    int n_valid = board_valid_spots.size();
    if(depth == 5 || n_valid <= 0){//reach 2 step or game end
        return count_value(board);
    }
    if(cur_player == player){//max player
        value = -9999;
        for(Point max_p: board_valid_spots){
            Board next = predict_board(board, max_p, cur_player);
            value = max(value, minimax(next, depth + 1, 3 - cur_player));
        }
        return value;
    }else{//min player
        value = 9999;
        for(Point min_p: board_valid_spots){
            Board oppo_next = predict_board(board, min_p, cur_player);
            value = min(value, minimax(oppo_next, depth + 1, 3 - cur_player));
        }
        return value;
    }
}//minimax

void read_board(std::ifstream& fin) {
    fin >> player;
    opposite_player = 3 - player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
            //if(board[i][j] == player) player_disc++;
            //if(board[i][j] == opposite_player) opposite_disc++;
        }
    }
}


void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void find_spot_value(Board b, int player){
    for(Point vp: next_valid_spots){
        Board next = predict_board(b, vp, player);
        int board_value = minimax(next, 1, 3 - player);
        spots_value.push_back(board_value);
    }
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    //find the max path
    int index = 0;
    int value = -9999;
    for(int i = 0; i < n_valid_spots; i++){
        if(spots_value[i] >= value){
            value = spots_value[i];
            index = i;
        }
    }
    Point p = next_valid_spots[index];
    if(!is_spot_valid(board, p, player)){//prevent invalid move
        p = next_valid_spots[0];
    }
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    find_spot_value(board, player);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

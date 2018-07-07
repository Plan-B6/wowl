// Chess++.cpp : Defines the entry point for the console application.
// TODO
/*
Fix undo for en passant
Fix undo for AI
Check game states (checkmate, stalemate etc.)
Check for threefold repitition
Check for fifty move rule
Promotion UI
Underpromotion
Refactor code
Replace defines with enums
Let player choose which side to play as

//PRIORITY
Transposition tables
Move ordering
*/

#include "stdafx.h"
#include "Wowl.h"
#include <SFML/Graphics.hpp>

#define SIZE 64
#define AI_TURN BLACK

std::vector<std::string> posVec;
Board b;
sf::Sprite spr_piece[32];
int spr_board[BOARD_SIZE][BOARD_SIZE] =
{ 
{-5,-4,-3,-2,-1,-3,-4,-5 },
{-6,-6,-6,-6,-6,-6,-6,-6 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 6, 6, 6, 6, 6, 6, 6, 6 },
{ 5, 4, 3, 2, 1, 3, 4, 5 }
};

//Convert to chess notation (float vector)
std::string toNotation(sf::Vector2f pos) {
	std::string s = "  ";
	s[0] = char(pos.x / SIZE + 97);
	s[1] = char(7 - pos.y / SIZE + 49);
	return s;
}
//Convert to chess notation (int vector)
std::string toNotation(sf::Vector2i pos) {
	std::string s = "  ";
	s[0] = char(pos.x / SIZE + 97);
	s[1] = char(7 - pos.y / SIZE + 49);
	return s;
}
//Convert to coordinates
sf::Vector2f toCoord(char a, char b) {
	int x = int(a) - 97;
	int y = 7 - int(b) + 49;
	return sf::Vector2f(x * SIZE, y * SIZE);
}
//Move sprite
void moveSprite(std::string s) {
	sf::Vector2f oldPos = toCoord(s[0], s[1]);
	sf::Vector2f newPos = toCoord(s[2], s[3]);

	//Capture --> put piece sprite out of window
	for (int i = 0; i < 32; i++) {
		if (spr_piece[i].getPosition() == newPos) {
			spr_piece[i].setPosition(-999, -999);
		}
	}

	//Move piece
	for (int i = 0; i < 32; i++) {
		if (spr_piece[i].getPosition() == oldPos) {
			spr_piece[i].setPosition(newPos);
		}
	}
}
//Defines special sprite behavior for en passant, castling and promotion
void specialMoveSprite(sf::Vector2f newVec, int oldc, int newc) {
	for (int i = 0; i < 32; i++) {
		//White en passant
		if (spr_piece[i].getPosition().y == newVec.y + SIZE && spr_piece[i].getPosition().x == newVec.x && b.getSquarePiece(newc) == WP) {
			if (oldc <= 58 && oldc >= 51 && b.getSquarePiece(newc - 10) == 0) {
				spr_piece[i].setPosition(-999, -999);
			}
		}
		//Black en passant
		else if (spr_piece[i].getPosition().y == newVec.y - SIZE && spr_piece[i].getPosition().x == newVec.x && b.getSquarePiece(newc) == BP) {
			if (oldc <= 68 && oldc >= 61 && b.getSquarePiece(newc + 10) == 0) {
				spr_piece[i].setPosition(-999, -999);
			}
		}
		//Short castling
		if (spr_piece[i].getPosition().x == newVec.x + SIZE && spr_piece[i].getPosition().y == newVec.y && abs(b.getSquarePiece(newc)) == WK && oldc - newc == -2) {
			spr_piece[i].setPosition(newVec.x - SIZE, newVec.y);
		}
		//Long castling
		else if (spr_piece[i].getPosition().x == newVec.x - 2 * SIZE && spr_piece[i].getPosition().y == newVec.y && abs(b.getSquarePiece(newc)) == WK && oldc - newc == 2) {
			spr_piece[i].setPosition(newVec.x + SIZE, newVec.y);
		}
		//White promotion (to queen)
		if (spr_piece[i].getPosition().y == newVec.y && spr_piece[i].getPosition().x == newVec.x && b.getSquarePiece(newc) == WQ) {
			if (newc <= 28 && newc >= 21 && oldc <= 38 && oldc >= 31) {
				spr_piece[i].setTextureRect(sf::IntRect(SIZE, 0, SIZE, SIZE));
			}
		}
		//Black promotion (to queen)
		else if (spr_piece[i].getPosition().y == newVec.y && spr_piece[i].getPosition().x == newVec.x && b.getSquarePiece(newc) == BQ) {
			if (newc <= 98 && newc >= 91 && oldc <= 88 && oldc >= 81) {
				spr_piece[i].setTextureRect(sf::IntRect(SIZE, SIZE, SIZE, SIZE));
			}
		}
	}
}
//Load position
void loadPosition() {
	//Load starting position
	int sprcount = 0;
	for (int a = 0; a < BOARD_SIZE; a++) {
		for (int b = 0; b < BOARD_SIZE; b++) {
			int i = spr_board[b][a];
			if (!i) continue;
			int x = abs(i) - 1;
			int y = i > 0 ? 0 : 1;
			spr_piece[sprcount].setTextureRect(sf::IntRect(SIZE * x, SIZE * y, SIZE, SIZE));
			spr_piece[sprcount].setPosition(SIZE * a, SIZE * b);
			sprcount++;
		}
	}

	//Load moves from existing position
	for (int i = 0; i < posVec.size(); i++) {
		moveSprite(posVec[i]);
		sf::Vector2f oldPos = toCoord(posVec[i][0], posVec[i][1]);
		sf::Vector2f newPos = toCoord(posVec[i][2], posVec[i][3]);
		std::string piecestr = toNotation(oldPos);
		int oldcoord = b.convertCoord(b.toCoord(piecestr[0], piecestr[1]));
		piecestr = toNotation(newPos);
		int newcoord = b.convertCoord(b.toCoord(piecestr[0], piecestr[1]));
		//Special moves
		specialMoveSprite(newPos, oldcoord, newcoord);

	}

	//Set board position
	b.setPosition(posVec);
}
int main()
{
	/*INITIALIZATION*/
	b.reserveVectors();
	Evaluation currentEval;
	Wowl AI;

	sf::RenderWindow window(sf::VideoMode(512, 512), "Chess++", sf::Style::Titlebar | sf::Style::Close);

	/*TEXTURES AND SPRITES*/
	sf::Texture tp;
	tp.loadFromFile("images/pieces.png");
	sf::Texture tb;
	tb.loadFromFile("images/board.png");
	sf::Sprite spr_board(tb);
	for (int i = 0; i < 32; i++) {
		spr_piece[i].setTexture(tp);
	}
	loadPosition();

	/*VARIABLES*/
	//Drag and drop
	bool mouse_drag = false;
	float drag_x, drag_y = 0;
	int drag_piece = 0;
	sf::Vector2f oldPos;

	while (window.isOpen()) {

		sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		sf::Event e;
		while (window.pollEvent(e)) {

			if (e.type == sf::Event::Closed) {
				window.close();
			}

			//Undo move
			if (e.type == sf::Event::KeyPressed) {
				if (e.key.code == sf::Keyboard::BackSpace) {
					if (posVec.size() > 0) {
						posVec.pop_back();
						std::cout << "Undo move" << std::endl;
						loadPosition();
					}
				}
			}

			if (b.getTurn() != AI_TURN) {
				//Drag and drop
				if (e.type == sf::Event::MouseButtonPressed) {
					if (e.key.code == sf::Mouse::Left) {
						for (int i = 0; i < 32; i++) {
							if (spr_piece[i].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
								//Check for turn
								std::string piecestr = toNotation(spr_piece[i].getPosition());
								int currentcoord = b.convertCoord(b.toCoord(piecestr[0], piecestr[1]));
								if (b.getSquarePiece(currentcoord) > 0 && b.getTurn() == BLACK) { break; }
								if (b.getSquarePiece(currentcoord) < 0 && b.getTurn() == WHITE) { break; }

								//Drag piece
								mouse_drag = true;
								drag_piece = i;
								drag_x = mousePos.x - spr_piece[drag_piece].getPosition().x;
								drag_y = mousePos.y - spr_piece[drag_piece].getPosition().y;
								//Save current position
								oldPos = spr_piece[drag_piece].getPosition();
							}
						}
					}
				}
				if (e.type == sf::Event::MouseButtonReleased) {
					if (e.key.code == sf::Mouse::Left && mouse_drag) {
						mouse_drag = false;
						//Fix new position to nearest square
						sf::Vector2f factor = spr_piece[drag_piece].getPosition() + sf::Vector2f(SIZE / 2, SIZE / 2);
						sf::Vector2f newPos = sf::Vector2f(SIZE * int(factor.x / SIZE), SIZE * int(factor.y / SIZE));

						/*LEGALITY CHECK*/
						std::string piecestr = toNotation(oldPos);
						int oldcoord = b.convertCoord(b.toCoord(piecestr[0], piecestr[1]));
						piecestr = toNotation(newPos);
						int newcoord = b.convertCoord(b.toCoord(piecestr[0], piecestr[1]));
						if (b.checkLegal(oldcoord, newcoord)) {
							//Move sprite
							moveSprite(toNotation(oldPos) + toNotation(newPos));
							spr_piece[drag_piece].setPosition(newPos);
							//Save move only if piece position has changed
							if (toNotation(oldPos) != toNotation(newPos)) {
								posVec.emplace_back(toNotation(oldPos) + toNotation(newPos));
								b.move(oldcoord, newcoord);
								specialMoveSprite(newPos, oldcoord, newcoord);

								b.outputBoard();
								std::cout << currentEval.totalEvaluation(b, WHITE) << std::endl << std::endl;
							}
						}
						else {
							//Move sprite to old position
							spr_piece[drag_piece].setPosition(oldPos);
						}
					}
				}
				//Update piece sprite position
				if (mouse_drag) { spr_piece[drag_piece].setPosition(mousePos.x - drag_x, mousePos.y - drag_y); }
			}
			else if (b.getTurn() == AI_TURN) {
				//Fix player piece to nearest square
				sf::Vector2f playerFactor = spr_piece[drag_piece].getPosition() + sf::Vector2f(SIZE / 2, SIZE / 2);
				sf::Vector2f playerNewPos = sf::Vector2f(SIZE * int(playerFactor.x / SIZE), SIZE * int(playerFactor.y / SIZE));
				spr_piece[drag_piece].setPosition(playerNewPos);

				//Make move
				AI.negaMax(b, SEARCH_DEPTH, AI_TURN, -WIN_SCORE, WIN_SCORE);
				std::cout << AI.bestMove.x << " " << AI.bestMove.y << std::endl;
				b.move(AI.bestMove.x, AI.bestMove.y);
				sf::Vector2i oldPos = b.convertCoord(AI.bestMove.x);
				sf::Vector2i newPos = b.convertCoord(AI.bestMove.y);

				//Multiply vectors with SIZE factor
				oldPos *= SIZE;
				newPos *= SIZE;

				//Search for sprite
				for (int i = 0; i < 32; i++) {
					if (spr_piece[i].getGlobalBounds().contains(oldPos.x, oldPos.y)) {
						drag_piece = i;
					}
				}

				//Move sprite
				sf::Vector2f newPosF = static_cast<sf::Vector2f>(newPos);
				moveSprite(toNotation(oldPos) + toNotation(newPos));
				spr_piece[drag_piece].setPosition(newPosF);
				specialMoveSprite(newPosF, AI.bestMove.x, AI.bestMove.y);

				posVec.emplace_back(toNotation(oldPos) + toNotation(newPos));
				b.outputBoard();
				std::cout << currentEval.totalEvaluation(b, WHITE) << std::endl << std::endl;
			}

		}
		
		/*DRAW*/
		window.clear(sf::Color::Black);
		window.draw(spr_board);
		for (int i = 0; i < 32; i++) {
			window.draw(spr_piece[i]);
		}
		window.display();	
	}

    return 0;
}
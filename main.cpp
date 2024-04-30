#include <iostream>
#include <vector>
#include <algorithm>

#include <raylib.h>
#include <raymath.h>

#define G	500

bool Colliding(Rectangle &a, Rectangle &b) {
	if (b.x + b.width > a.x &&
      b.y + b.height > a.y &&
	    a.x + a.width > b.x &&
	    a.y + a.height > b.y)
		return true;
	else return false;
}

namespace dddd {
	class Rectangle : public ::Rectangle {
	public:
		int Top() {
			return y;
		}

		int Bottom() {
			return y + height;
		}

		int Left() {
			return x;
		}

		int Right() {
			return x + width;
		}

		bool Above(Rectangle r) {
			return Bottom() <= r.Top();
		}

		Rectangle(::Rectangle r) : ::Rectangle(r) {}
	};

}

typedef enum {
  LEFT, RIGHT
} directions;

class Entity {
public:
	dddd::Rectangle bounds;
	Vector2 velocity;
	float maxSpeed = 200.0f;
	Color color;
	bool moving;
	bool onGround;
	int direction;
	int health;

	Entity(Rectangle b, Color c) :
		bounds(b),
		velocity({ 0.0f, 0.0f }),
		color(c),
		moving(0),
		onGround(0),
		direction(directions::LEFT),
		health(100)
		{}

	void UpdateBounds(std::vector<dddd::Rectangle> &walls) {
		float deltaTime = GetFrameTime();

		dddd::Rectangle nextBounds { bounds };

		nextBounds.y += velocity.y*deltaTime;
		velocity.y += G*deltaTime;

		for (size_t i = 0; i < walls.size(); ++i) {
			if (Colliding(nextBounds, walls[i])) {
				if (bounds.Above(walls[i])) {
					onGround = true;
					nextBounds.y = walls[i].y - bounds.height;
				} else {
					nextBounds.y = walls[i].y + walls[i].height;
				}
				velocity.y = 0.0f;
				break;
			}
		}

		if (!moving && velocity.x != 0.0f) {
			const float friction = 20.0f;
			velocity.x += (velocity.x > 0.0f ? -1.0f : 1.0f) * friction;
		}

		nextBounds.x += velocity.x*deltaTime;

		for (size_t i = 0; i < walls.size(); ++i) {
			if (Colliding(nextBounds, walls[i])) {
				if (bounds.Left() >= walls[i].Right()) nextBounds.x = walls[i].Right();
				else                                   nextBounds.x = walls[i].Left() - bounds.width;
				velocity.x = 0;
				break;
			}
		}

		bounds = nextBounds;
	}
};

class Player : public Entity {
public:
	bool attacking;
	int aframes;
	dddd::Rectangle weaponBounds;

#define PLAYER_WIDTH	20
#define PLAYER_HEIGHT	60
#define PLAYER_COLOR	WHITE
	Player() :
		Entity( { 0, 0, PLAYER_WIDTH, PLAYER_HEIGHT }, PLAYER_COLOR ),
		attacking(false),
		aframes(0),
		weaponBounds({0,0,0,0})
		{}

	Player(float x, float y) : 
		Entity( { x, y, PLAYER_WIDTH, PLAYER_HEIGHT }, PLAYER_COLOR ),
		attacking(false),
		aframes(0),
		weaponBounds({0,0,0,0})
		{}

	void initiateAttack() {
		if (!aframes) {
			aframes = 15;
			attacking = true;
		}
	}

	void Move() {
		//std::cout << "player.velocity.x = " << velocity.x << '\n';
		if (IsKeyDown(KEY_SPACE) && onGround) {
#define PLAYER_JUMP_SPD	325.0f
			velocity.y = -PLAYER_JUMP_SPD;
			onGround = false;
#undef PLAYER_JUMP_SPD
		}
		const float hAccel = 30.0f;
		if (IsKeyDown(KEY_A)) {
			moving = true;
			if (!attacking) direction = directions::LEFT;
			//std::cout << directions::LEFT << '\n';
			if (velocity.x - hAccel > -maxSpeed) velocity.x -= hAccel;
			else                                 velocity.x  = -maxSpeed;
		}
		if (IsKeyDown(KEY_D)) {
			moving = true;
			if (!attacking) direction = directions::RIGHT;
			//std::cout << directions::RIGHT << '\n';
			if (velocity.x + hAccel < maxSpeed) velocity.x += hAccel;
			else                                velocity.x  = maxSpeed;
		}
		if (IsKeyReleased(KEY_A) || IsKeyReleased(KEY_D)) {
			moving = false;
		}
		if (IsKeyPressed(KEY_J)) {
			initiateAttack();
		}

	}

	void Update() {
		if (aframes) {
			--aframes;
			weaponBounds = bounds;
			weaponBounds.height = 5.0f;
			weaponBounds.width = 50.0f;
			weaponBounds.y += 25.0f;
			if (direction == directions::LEFT) weaponBounds.x -= weaponBounds.width;
			if (direction == directions::RIGHT) weaponBounds.x += bounds.width;
		}
	}

	void DrawWeapon() {
		dddd::Rectangle hilt = weaponBounds;
		hilt.width = 5.0f;
		hilt.height = 25.0f;
		hilt.y -= 10.0f;
		//maybe I can combine this with Update directional code...
		if (direction == directions::LEFT) hilt.x += weaponBounds.width - 15.0f;
		if (direction == directions::RIGHT) hilt.x += 10.0f;
		DrawRectangleRec(weaponBounds, GREEN);
		DrawRectangleRec(hilt, GREEN);
		std::cout << "Drawing~!\n";
	}
};

class Monster : public Entity {
public:
	bool alive;

	Monster(float x, float y) : 
		Entity( { x, y, PLAYER_WIDTH, PLAYER_HEIGHT }, RED ),
		alive(true)
		{}

private:
};

void InitializeWindow(Vector2 screenSize) {
	InitWindow(screenSize.x, screenSize.y, "FUN -- ?");
}

void InitializeCamera(Camera2D &camera, Player &player, Vector2 screenSize) {
	camera = { 0 };
	camera.target = (Vector2){ 0, 0 };
	camera.target = (Vector2){ 0, 0 };
	camera.rotation = 0.0f;
	//camera.zoom = 1.0f;
	camera.zoom = 2.0f/3.0f;
}

// std::vector <dddd::Rectangle> four_walls {
// 		dddd::Rectangle({ 0, 0, 1200, 5 }),
// 		dddd::Rectangle({ 0, 0, 5, 900 }),
// 		dddd::Rectangle({ 0, 895, 1200, 5 }),
// 		dddd::Rectangle({ 1195, 0, 5, 900 }),
// };

#define four_walls	dddd::Rectangle({ 0, 0, 1200, 5 }), \
  dddd::Rectangle({ 0, 0, 5, 900 }), \
	dddd::Rectangle({ 0, 895, 1200, 5 }), \
	dddd::Rectangle({ 1195, 0, 5, 900 })

std::vector <dddd::Rectangle> test_level { 
	four_walls,
		 dddd::Rectangle({ 0, 100, 600, 100 }),
		 dddd::Rectangle({ 700, 200, 500, 100 }),
		//dddd::Rectangle({ 0, 300, 1200, 100 }),

		//floor
		dddd::Rectangle({ 0, 800, 1200, 100 }),
};

int main() {
	//Vector2 screenSize = { 1200, 900 };
	Vector2 screenSize = { 800, 600 };
	InitializeWindow(screenSize);

	Player player;
//	player.bounds.x += 650;
	player.bounds.x += 10;

	Camera2D camera;
	InitializeCamera(camera, player, screenSize);

	// std::vector<dddd::Rectangle> level_walls { 
	// 	// dddd::Rectangle({ 0, 100, 600, 100 }),
	// 	// dddd::Rectangle({ 700, 200, 500, 100 }),
	// 	//dddd::Rectangle({ 0, 300, 1200, 100 }),

	// 	//floor
	// 	dddd::Rectangle({ 0, 800, 1200, 100 }),
	// 	//walls
	// 	dddd::Rectangle({ 0, 0, 1200, 5 }),
	// 	dddd::Rectangle({ 0, 0, 5, 900 }),
	// 	dddd::Rectangle({ 0, 895, 1200, 5 }),
	// 	dddd::Rectangle({ 1195, 0, 5, 900 }),
	// };
	std::vector<dddd::Rectangle> &level_walls = test_level;

	std::vector<Monster> monsters {
		Monster(500, 40),
	};

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(DARKGRAY);
		BeginMode2D(camera);



		player.Move();
		player.UpdateBounds(level_walls);
		player.Update();
		DrawRectangleRec(player.bounds, player.color);


		bool monstersDied;
		
		if (player.aframes) {
			player.DrawWeapon();
			for (auto &m : monsters) {
				if (Colliding(player.weaponBounds, m.bounds)) {
					//m.color = PINK;
					std::cout << "hit monster at " << m.bounds.x << ',' << m.bounds.y << '\n';
					m.alive = false;

					monstersDied = true;
				}
			}
		}
		else {
			player.attacking = false;
			player.weaponBounds = dddd::Rectangle({0,0,0,0});
		}

		for (auto &r : level_walls) {
			DrawRectangleRec(r, LIGHTGRAY);
		}


		if (monstersDied) {
			//erase all monsters that are not alive
			monsters.erase(std::remove_if(monsters.begin(), monsters.end(),
				[](Monster m) {
					return !m.alive;
				}
			), monsters.end());
		}

		for (auto &m : monsters) {
			DrawRectangleRec(m.bounds, m.color);
		}

		EndMode2D();
		EndDrawing();
	}

	return 0;
}

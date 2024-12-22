#pragma once

#ifdef RUNENGINE_EXPORTS
#define RUN_API __declspec(dllexport)
#else
#define RUN_API __declspec(dllimport)
#endif

#include <random>
#include <vector>

constexpr float scr_width{ 800.0f };
constexpr float scr_height{ 600.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 550.0f };

constexpr char evil1_type{ 0b00000001 };
constexpr char evil2_type{ 0b00000010 };
constexpr char evil3_type{ 0b00000100 };
constexpr char evil4_type{ 0b00001000 };
constexpr char evil5_type{ 0b00010000 };
constexpr char hero_type{ 0b00100000 };

constexpr char stop_flag{ 0b00000000 };
constexpr char run_flag{ 0b00000001 };
constexpr char jump_up_flag{ 0b00000010 };
constexpr char jump_down_flag{ 0b00000100 };
constexpr char fall_flag{ 0b00001000 };

enum class dirs { left = 0, right = 1, up = 2, down = 3 };

namespace dll
{
	class RUN_API RANDiT
	{
		private:
			std::seed_seq* sq{ nullptr };
			std::random_device rd{};
			std::vector<int>vSeeds;
			std::mt19937* twister{ nullptr };

		public:
			
			RANDiT();
			~RANDiT();

			int operator() (int min, int max);
	};

	class RUN_API PROTON
	{
		protected:
			float width = 0;
			float height = 0;

		public:
			float x{ 0 };
			float y{ 0 };
			float ex{ 0 };
			float ey{ 0 };

			PROTON(float _x = 0, float _y = 0, float _width = 1.0f, float _height = 1.0f);
			virtual ~PROTON() {};

			void SetEdges();
			void NewDims(float _new_width, float _new_height);
			float GetWidth() const;
			float GetHeight() const;
	};

	class RUN_API PROT_MESH
	{
		private:
			PROTON* base_pointer{ nullptr };
			size_t mArraySize{ 0 };
			size_t current_position{ 0 };
			size_t last_position{ 0 };
			bool validated = false;

		public:
			PROT_MESH(size_t array_size);
			~PROT_MESH();

			bool is_vaild() const;
			size_t size() const;
			void push_back(PROTON& element);
			void push_front(PROTON& element);
			PROTON& operator[](size_t index);
			void operator() (size_t index, PROTON& element);
	};

	class RUN_API BASE_CREATURE :public PROTON
	{
		protected:
			float speed{};

			float move_sx{};
			float move_sy{};
			float move_ex{};
			float move_ey{};

			bool hor_line{ false };
			bool vert_line{ false };

			float intercept{};
			float slope{};

			char type_flag{ 0 };
			char move_flag{ 0 };

			int strenght{ 0 };
			int attack_delay{ 0 };

			int frame{ 0 };
			int max_frames{ 0 };
			int frame_delay{ 0 };

		public:
			int lifes{};
			dirs dir{ dirs::left };

			BASE_CREATURE(char what_type, float _sx, float _sy);
			virtual ~BASE_CREATURE() {};

			void SetPathInfo(float _dest_x, float _dest_y);
			
			int GetFrame();
			int Attack();
			
			char GetTypeFlag() const;
			char GetMoveFlag() const;
			
			bool CheckTypeFlag(char which_flag) const;
			void SetTypeFlag(char which_flag);
			void NullTypeFlag(char which_flag);

			bool CheckMoveFlag(char which_flag) const;
			void SetMoveFlag(char which_flag);
			void NullMoveFlag(char which_flag);

			virtual float GetXAxisMove(float gear) const = 0;
			virtual char Move(float gear, float dest_x, float dest_y, PROT_MESH& platforms) = 0;
			virtual char Jump(float gear, PROT_MESH& platforms) = 0;
			virtual char Fall(float gear, PROT_MESH& platforms) = 0;
			virtual void Release() = 0;
	};

	class RUN_API HERO :public BASE_CREATURE
	{
		protected:
			HERO(float _where_x, float _where_y);

		public:

			friend RUN_API BASE_CREATURE* CreatureFactory(char what_type, float start_x, float start_y);

			float GetXAxisMove(float gear) const override;
			char Move(float gear, float dest_x, float dest_y, PROT_MESH& platforms) override;
			char Jump(float gear, PROT_MESH& platforms) override;
			char Fall(float gear, PROT_MESH& platforms) override;
			void Release() override;
	};

	class RUN_API EVIL :public BASE_CREATURE
	{
	protected:
		EVIL(char what, float _where_x, float _where_y);

	public:

		friend RUN_API BASE_CREATURE* CreatureFactory(char what_type, float start_x, float start_y);

		float GetXAxisMove(float gear) const override;
		char Move(float gear, float dest_x, float dest_y, PROT_MESH& platforms) override;
		char Jump(float gear, PROT_MESH& platforms) override;
		char Fall(float gear, PROT_MESH& platforms) override;
		void Release() override;
	};

	class RUN_API SHOT :public PROTON
	{
	private:
		float move_sx = 0;
		float move_sy = 0;
		float move_ex = 0;
		float move_ey = 0;
		float intercept = 0;
		float slope = 0;
		bool hor_line = false;
		bool vert_line = false;

	public:

		SHOT(float init_x, float init_y, float target_x, float target_y);

		bool Move(float gear);
	};

	/////////////////////////////////////////
	typedef BASE_CREATURE* Creature;

	RUN_API Creature CreatureFactory(char what_type, float start_x, float start_y);
}
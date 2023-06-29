#include "Chipotto.h"

namespace chipotto
{
	Emulator::Emulator()
	{
		KeyboardMap[SDLK_1] = 0x0;
		KeyboardMap[SDLK_2] = 0x1;
		KeyboardMap[SDLK_3] = 0x2;
		KeyboardMap[SDLK_4] = 0x3;
		KeyboardMap[SDLK_q] = 0x4;
		KeyboardMap[SDLK_w] = 0x5;
		KeyboardMap[SDLK_e] = 0x6;
		KeyboardMap[SDLK_r] = 0x7;
		KeyboardMap[SDLK_a] = 0x8;
		KeyboardMap[SDLK_s] = 0x9;
		KeyboardMap[SDLK_d] = 0xA;
		KeyboardMap[SDLK_f] = 0xB;
		KeyboardMap[SDLK_z] = 0xC;
		KeyboardMap[SDLK_x] = 0xD;
		KeyboardMap[SDLK_c] = 0xE;
		KeyboardMap[SDLK_v] = 0xF;

		for (const auto& pair : KeyboardMap)
		{
			KeyboardValuesMap[pair.second] = static_cast<SDL_Scancode>(pair.first);
		}

		Opcodes[0x0] = std::bind(&Emulator::Opcode0, this, std::placeholders::_1);
		Opcodes[0x1] = std::bind(&Emulator::Opcode1, this, std::placeholders::_1);
		Opcodes[0x2] = std::bind(&Emulator::Opcode2, this, std::placeholders::_1);
		Opcodes[0x3] = std::bind(&Emulator::Opcode3, this, std::placeholders::_1);
		Opcodes[0x4] = std::bind(&Emulator::Opcode4, this, std::placeholders::_1);
		Opcodes[0x5] = std::bind(&Emulator::Opcode5, this, std::placeholders::_1);
		Opcodes[0x6] = std::bind(&Emulator::Opcode6, this, std::placeholders::_1);
		Opcodes[0x7] = std::bind(&Emulator::Opcode7, this, std::placeholders::_1);
		Opcodes[0x8] = std::bind(&Emulator::Opcode8, this, std::placeholders::_1);
		Opcodes[0x9] = std::bind(&Emulator::Opcode9, this, std::placeholders::_1);
		Opcodes[0xA] = std::bind(&Emulator::OpcodeA, this, std::placeholders::_1);
		Opcodes[0xB] = std::bind(&Emulator::OpcodeB, this, std::placeholders::_1);
		Opcodes[0xC] = std::bind(&Emulator::OpcodeC, this, std::placeholders::_1);
		Opcodes[0xD] = std::bind(&Emulator::OpcodeD, this, std::placeholders::_1);
		Opcodes[0xE] = std::bind(&Emulator::OpcodeE, this, std::placeholders::_1);
		Opcodes[0xF] = std::bind(&Emulator::OpcodeF, this, std::placeholders::_1);

		//FINISH IMPLEMENTATION OF SPRITES
		MemoryMapping[0x0] = 0xF0;
		MemoryMapping[0x1] = 0x90;
		MemoryMapping[0x2] = 0x90;
		MemoryMapping[0x3] = 0x90;
		MemoryMapping[0x4] = 0xF0;

		Window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * 10, height * 10, 0);
		if (!Window)
		{
			SDL_Log("Unable to create window: %s", SDL_GetError());
			return;
		}
		Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!Renderer)
		{
			SDL_Log("Unable to create renderer: %s", SDL_GetError());
			SDL_DestroyWindow(Window);
			return;
		}
		Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
		if (!Texture)
		{
			SDL_Log("Unable to create texture: %s", SDL_GetError());
			SDL_DestroyRenderer(Renderer);
			SDL_DestroyWindow(Window);
			return;
		}
	}

	bool Emulator::LoadFromFile(std::filesystem::path Path)
	{
		std::ifstream file;
		file.open(Path, std::ios::binary);
		if (!file.is_open()) return false;

		auto file_size = std::filesystem::file_size(Path);

		file.read(reinterpret_cast<char*>(MemoryMapping.data() + PC), file_size);
		file.close();
	}

	bool Emulator::Tick()
	{
		uint64_t tick = SDL_GetTicks64();

		if (DelayTimer > 0 && tick >= DeltaTimerTicks)
		{
			DelayTimer--;
			DeltaTimerTicks = 17 + SDL_GetTicks64();
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)
			{
				SDL_Keycode key = event.key.keysym.sym;
				if (KeyboardMap.contains(key))
				{
					Registers[WaitForKeyboardRegister_Index] = KeyboardMap[key];
					Suspended = false;
					PC += 2;
				}
			}
			if (event.type == SDL_QUIT)
			{
				return false;
			}
		}
		SDL_PumpEvents();

		if (Suspended) return true;

		uint16_t opcode = MemoryMapping[PC + 1] + (static_cast<uint16_t>(MemoryMapping[PC]) << 8);
		std::cout << std::hex << "0x" << PC << ": 0x" << opcode << "  -->  ";

		OpcodeStatus status = Opcodes[opcode >> 12](opcode);

		std::cout << std::endl;
		if (status == OpcodeStatus::IncrementPC)
		{
			PC += 2;
		}
		return status != OpcodeStatus::NotImplemented && status != OpcodeStatus::StackOverflow && status != OpcodeStatus::Error;
	}

	bool Emulator::IsValid() const
	{
		if (!Window || !Renderer || !Texture)
			return false;
		return true;
	}

	OpcodeStatus Emulator::Opcode0(const uint16_t opcode)
	{
		if ((opcode & 0xFF) == 0xE0)
		{
			std::cout << "CLS";
			uint8_t* pixels = nullptr;
			int pitch;
			int result = SDL_LockTexture(Texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
			memset(pixels, 0, pitch * height);
			SDL_UnlockTexture(Texture);
			SDL_RenderCopy(Renderer, Texture, nullptr, nullptr);
			SDL_RenderPresent(Renderer);
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0xEE)
		{
			if (SP > 0xF && SP < 0xFF) return OpcodeStatus::StackOverflow;
			std::cout << "RET";
			PC = Stack[SP & 0xF];
			SP -= 1;
			return OpcodeStatus::IncrementPC;
		}
		return OpcodeStatus::NotImplemented;
	}

	OpcodeStatus Emulator::Opcode1(const uint16_t opcode)
	{
		uint16_t address = opcode & 0x0FFF;
		std::cout << "JP 0x" << address;
		PC = address - 2;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::Opcode2(const uint16_t opcode)
	{
		uint16_t address = opcode & 0xFFF;
		std::cout << "CALL 0x" << (int)address;
		if (SP > 0xF)
		{
			SP = 0;
		}
		else
		{
			if (SP < 0xF)
			{
				SP += 1;
			}
			else
			{
				return OpcodeStatus::StackOverflow;
			}
		}
		Stack[SP] = PC;
		PC = address;
		return OpcodeStatus::NotIncrementPC;
	}

	OpcodeStatus Emulator::Opcode3(const uint16_t opcode)
	{
		uint8_t register_index = (opcode >> 8) & 0xF;
		uint8_t value = opcode & 0xFF;
		std::cout << "SE V" << (int)register_index << ", 0x" << (int)value;
		if (Registers[register_index] == value)
			PC += 2;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::Opcode4(const uint16_t opcode)
	{
		uint8_t register_index = (opcode >> 8) & 0xF;
		uint8_t value = opcode & 0xFF;
		std::cout << "SNE V" << (int)register_index << ", 0x" << (int)value;
		if (Registers[register_index] != value)
			PC += 2;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::Opcode5(const uint16_t opcode) //Skip next instruction if Vx = Vy. The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
	{
		uint8_t register_indexX = (opcode >> 8) & 0xF;
		uint8_t register_indexY = (opcode >> 4) & 0xF;
		std::cout << "SE V" << (int)register_indexX << ", V" << (int)register_indexY;
		if (Registers[register_indexX] == Registers[register_indexY])
			PC += 2;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::Opcode6(const uint16_t opcode)
	{
		uint8_t register_index = (opcode >> 8) & 0xF;
		uint8_t register_value = opcode & 0xFF;
		Registers[register_index] = register_value;
		std::cout << "LD V" << (int)register_index << ", 0x" << (int)register_value;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::Opcode7(const uint16_t opcode)
	{
		uint8_t register_index = (opcode >> 8) & 0xF;
		uint8_t value = opcode & 0xFF;
		std::cout << "ADD V" << (int)register_index << ", 0x" << (int)value;
		Registers[register_index] += value;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::Opcode8(const uint16_t opcode)
	{
		if ((opcode & 0xF) == 0x0)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			Registers[registerX_index] = Registers[registerY_index];
			std::cout << "LD V" << (int)registerX_index << ", V" << (int)registerY_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0x1)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			Registers[registerX_index] |= Registers[registerY_index];
			std::cout << "OR V" << (int)registerX_index << ", V" << (int)registerY_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0x2)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			Registers[registerX_index] &= Registers[registerY_index];
			std::cout << "AND V" << (int)registerX_index << ", V" << (int)registerY_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0x3)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			Registers[registerX_index] ^= Registers[registerY_index];
			std::cout << "XOR V" << (int)registerX_index << ", V" << (int)registerY_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0x4)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			int result = static_cast<int>(Registers[registerX_index]) + Registers[registerY_index];
			if (result > 255) Registers[0xF] = 1;
			else Registers[0xF] = 0;
			Registers[registerX_index] += Registers[registerY_index];
			std::cout << "ADD V" << (int)registerX_index << ", V" << (int)registerY_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0x5)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			if (Registers[registerX_index] > Registers[registerY_index]) Registers[0xF] = 1;
			else Registers[0xF] = 0;
			Registers[registerX_index] -= Registers[registerY_index];
			std::cout << "SUB V" << (int)registerX_index << ", V" << (int)registerY_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0x6)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			Registers[0xF] = Registers[registerX_index] & 0x1; 
			Registers[registerX_index] >>= 1;
			std::cout << "SHR V" << (int)registerX_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0x7)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			if (Registers[registerY_index] > Registers[registerX_index]) Registers[0xF] = 1;
			else Registers[0xF] = 0;
			Registers[registerX_index] = Registers[registerY_index] - Registers[registerX_index];
			std::cout << "SUBN V" << (int)registerX_index << ", V" << (int)registerY_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xF) == 0xE)
		{
			uint8_t registerX_index = (opcode >> 8) & 0xF;
			uint8_t registerY_index = (opcode >> 4) & 0xF;
			Registers[0xF] = Registers[registerX_index] >> 7;
			Registers[registerX_index] <<= 1;
			std::cout << "SHL V" << (int)registerX_index << "{, V" << (int)registerY_index << "}";
			return OpcodeStatus::IncrementPC;
		}
		else
		{
			std::cout << "Unknown opcode: " << std::hex << opcode;
			return OpcodeStatus::IncrementPC;
		}
	}

	OpcodeStatus Emulator::Opcode9(const uint16_t opcode) // Skip next instruction if Vx != Vy. The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
	{
		uint8_t registerX_index = (opcode >> 8) & 0xF;
		uint8_t registerY_index = (opcode >> 4) & 0xF;
		std::cout << "SNE V" << (int)registerX_index << ", V" << (int)registerY_index;
		if (Registers[registerX_index] != Registers[registerY_index])
			PC += 2;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::OpcodeA(const uint16_t opcode) // Set I = nnn. The value of register I is set to nnn.
	{
		uint16_t value = (opcode & 0xFFF);
		std::cout << "LD I, 0x" << (int)value;
		I = value;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::OpcodeB(const uint16_t opcode) // Jump to location nnn + V0. The program counter is set to nnn plus the value of V0.
	{
		uint16_t value = (opcode & 0xFFF);
		std::cout << "JP V0, 0x" << (int)value;
		PC = Registers[0] + value - 2;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::OpcodeC(const uint16_t opcode)
	{
		uint8_t register_index = (opcode >> 8) & 0xF;
		uint8_t random_mask = opcode & 0xFF;
		std::cout << "RND V" << (int)register_index << ", 0x" << (int)random_mask;
		Registers[register_index] = (std::rand() % 256) & random_mask;
		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::OpcodeD(const uint16_t opcode)
	{
		uint8_t registerX_index = (opcode >> 8) & 0xF;
		uint8_t registerY_index = (opcode >> 4) & 0xF;
		uint8_t sprite_height = opcode & 0xF;
		std::cout << "DRW V" << (int)registerX_index << ", V" << (int)registerY_index << ", " << (int)sprite_height;

		uint8_t x_coord = Registers[registerX_index] % width;
		uint8_t y_coord = Registers[registerY_index] % height;

		uint8_t* pixels = nullptr;
		int pitch;
		int result = SDL_LockTexture(Texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);
		if (result != 0)
		{
			SDL_Log("Failed to lock texture");
			return OpcodeStatus::Error;
		}

		for (int y = 0; y < sprite_height; ++y)
		{
			if (y + y_coord >= height) break;
			uint8_t row_byte = MemoryMapping[I + y];
			for (int x = 0; x < 8; x++)
			{
				uint8_t pixel_color = (row_byte >> (7 - x)) & 0x1;
				uint8_t color = 0;
				if (pixel_color)
				{
					color = 0xFF;
				}
				if (x + x_coord >= width) break;
				int pixel_index = (x + x_coord) * 4 + pitch * (y + y_coord);
				uint8_t existing_pixel = pixels[pixel_index];
				color ^= existing_pixel;

				if (existing_pixel != 0 && color != 0)
				{
					Registers[0xF] = 0x1;
				}

				pixels[pixel_index + 0] = color;
				pixels[pixel_index + 1] = color;
				pixels[pixel_index + 2] = color;
				pixels[pixel_index + 3] = color;
			}
		}

		SDL_UnlockTexture(Texture);

		SDL_RenderCopy(Renderer, Texture, nullptr, nullptr);
		SDL_RenderPresent(Renderer);

		return OpcodeStatus::IncrementPC;
	}

	OpcodeStatus Emulator::OpcodeE(const uint16_t opcode)
	{
		if ((opcode & 0xFF) == 0xA1)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "SKNP V" << (int)register_index;
			const uint8_t* keys_state = SDL_GetKeyboardState(nullptr);
			if (keys_state[KeyboardValuesMap[Registers[register_index]]] == 0)
			{
				PC += 2;
			}
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x9E)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "SKP V" << (int)register_index;
			const uint8_t* keys_state = SDL_GetKeyboardState(nullptr);
			if (keys_state[KeyboardValuesMap[Registers[register_index]]] == 1)
			{
				PC += 2;
			}
			return OpcodeStatus::IncrementPC;
		}
		return OpcodeStatus::NotImplemented;
	}

	OpcodeStatus Emulator::OpcodeF(const uint16_t opcode)
	{
		if ((opcode & 0xFF) == 0x55)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "LD [I], V" << (int)register_index;
			for (uint8_t i = 0; i < register_index; ++i)
			{
				MemoryMapping[I + i] = Registers[i];
			}
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x65)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "LD V" << (int)register_index << ", [I]";
			for (uint8_t i = 0; i < register_index; ++i)
			{
				Registers[i] = MemoryMapping[I + 1];
			}
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x33)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			uint8_t value = Registers[register_index];
			MemoryMapping[I] = value / 100;
			MemoryMapping[I + 1] = value - (MemoryMapping[I] * 100) / 10;
			MemoryMapping[I + 2] = value % 10;
			std::cout << "LD B, V" << (int)register_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x29)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "LD F, V" << (int)register_index;
			I = 5 * Registers[register_index];
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x0A)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "LD V" << (int)register_index << ", K";
			WaitForKeyboardRegister_Index = register_index;
			Suspended = true;
			return OpcodeStatus::WaitForKeyboard;
		}
		else if ((opcode & 0xFF) == 0x1E)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "ADD I, V" << (int)register_index;
			I += Registers[register_index];
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x18)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "LD ST, V" << (int)register_index;
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x15)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "LD DT, V" << (int)register_index;
			DelayTimer = Registers[register_index];
			DeltaTimerTicks = 17 + SDL_GetTicks64();
			return OpcodeStatus::IncrementPC;
		}
		else if ((opcode & 0xFF) == 0x07)
		{
			uint8_t register_index = (opcode >> 8) & 0xF;
			std::cout << "LD V" << (int)register_index << ", DT";
			Registers[register_index] = DelayTimer;
			return OpcodeStatus::IncrementPC;
		}
		else
		{
			return OpcodeStatus::NotImplemented;
		}
	}
	
	uint16_t Emulator::GetPCCounter() const
	{
		return PC;
	}

	uint8_t Emulator::GetSPCounter() const
	{
		return SP;
	}

	uint16_t Emulator::GetI() const
	{
		return I;
	}

	uint8_t Emulator::GetRegisters(const uint8_t index) const
	{
		return Registers[index];
	}

}

//int main(int argc, char** argv)
//{
//	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
//	{
//		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
//		return -1;
//	}
//
//	chipotto::Emulator emulator;
//
//	if (emulator.IsValid())
//	{
//		emulator.LoadFromFile("C:\\Users\\kavaf\\OneDrive\\AIV_2022-2023\\2022 - 2023 - 01 - LezioniProgetti\\TestEmulatorChip8\\PONG");
//		while (true)
//		{
//			if (!emulator.Tick())
//			{
//				break;
//			}
//		}
//	}
//
//	SDL_Quit();
//	return 0;
//}
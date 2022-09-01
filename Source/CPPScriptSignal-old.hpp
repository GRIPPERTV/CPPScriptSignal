#ifndef __Signal__like__RBXScriptSignal__
#define __Signal__like__RBXScriptSignal__

#include <vector>
#include <chrono>
#include <thread>
#include <functional>

template <typename... Parameters> class ScriptSignal {
private:
	std::vector<std::function<void(Parameters...)>> Connections;
	bool Idle{true}; // It's firing or not

	// Connection class
	class Handler {
	private:
		std::function<void()> ExternDisconnect;

	public:
		Handler(std::function<void()> const& Function) {
			// Receives an external function that disconnect (see Connect)
			ExternDisconnect = Function;
		};

		// Connection's function exists or not
		bool Connected{true};

		// Remove the function of connection
		void Disconnect() {
			if (!Connected) return;
			ExternDisconnect();
			Connected = false;
		}
	};

public:
	Handler Connect(std::function<void(Parameters...)> const& Function) {
		// Add function to Connections
		Connections.push_back(Function);

		// Creates a new connection class
		Handler Connection([&, Index = Connections.size() - 1]() {
			Connections.erase(Connections.begin() + Index);
		});

		// Return the initialized class
		return Connection;
	};

	// Yields the thread until the signal is fired
	int Wait() {
		std::chrono::steady_clock::time_point Time {std::chrono::steady_clock::now()};
		while (Idle) std::this_thread::yield();

		// Return the elapsed time to fire (milliseconds)
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - Time).count();
	};

	// Fire all connections
	void Fire(Parameters... Arguments) {
		if (Connections.size() == 0) return;

		Idle = false;
		for (auto const& Function : Connections)
			Function(Arguments...);

		Idle = true;
	};
};

#endif
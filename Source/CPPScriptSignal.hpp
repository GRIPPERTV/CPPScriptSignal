#ifndef CPPScriptSignal
#define CPPScriptSignal

#include <mutex>
#include <vector>
#include <chrono>
#include <thread>
#include <functional>
#include <condition_variable>

/** Alias for the default function type to be used */
#define f_(X) std::function<void(X)>

/**
 * @brief Class of signal
 *
 * @tparam Parameters The parameters to be used in function of connection
 */
template <typename... Parameters> class ScriptSignal {
protected:
	/** Struct of connection */
	struct Connection {
	protected:
		/**
		 * @brief Function to erase the connection's function
		 *
		 * @see ScriptSignal::Connection::Connection
		 */
		f_(void) Destroy;

		/**
		 * @brief Hold if connection's function still exists
		 *
		 * @see ScriptSignal::Connection::Connected
		 * @see ScriptSignal::Connection::Disconnect
		 */
		bool isConnected = true;

	public:
		/**
		 * @brief Constructor of connection to direct initialize Destroy function
		 *
		 * The Destroy function is used to call a out-scope member
		 * ScriptSignal::Functions to erase the function that
		 * the current connection holds, being initilized by a
		 * lambda reference that can call this member
		 *
		 * @see ScriptSignal::Connect
		 *
		 * @param Function Function or lambda to be used in Destroy
		 */
	 	Connection(const f_(void)& Function) : Destroy(Function) {}

	 	/**
	 	 * @brief Return if connection's function exists or not
		 *
		 * @return ScriptSignal::Connection::isConnected
		 */
		inline bool Connected() {
			return isConnected;
		}

		/**
		 * @brief Call Destroy and set isConnected to `false`
		 *
		 * After Destroy is called, the connection's function
		 * is erased and isConnected is set to `false`
		 *
		 * @note The function verify if isConnected is `true`
		 * before calling Destroy and change isConnected
		 *
		 * @see ScriptSignal::Connection::Destroy
		 * @see ScriptSignal::Connection::isConnected
		 */
		void Disconnect() {
			if (isConnected) {
				Destroy();
				isConnected = false;
			}
		}
	};

	/**
	 * @brief Manage the block of a thread by conditional statement
	 *
	 * @see ScriptSignal::Fire
	 * @see ScriptSignal::Wait
	 */
	std::condition_variable Condition;

	/**
	 * @brief A vector of all connections function
	 *
	 * @see ScriptSignal::Connect
	 * @see ScriptSignal::Fire
	 */
	std::vector<f_(Parameters...)> Functions;

	/**
	 * @brief A vector of all connections
	 *
	 * @see ScriptSignal::~ScriptSignal
	 * @see ScriptSignal::Connect
	 *
	 */
	std::vector<Connection*> Connections;

	/**
	 * @brief Access synchronization for Idle
	 *
	 * @see ScriptSignal::Fire
	 * @see ScriptSignal::Wait
	 */
	std::mutex Current;

	/**
	 * @brief Thread blocking condition
	 *
	 * @see ScriptSignal::Fire
	 * @see ScriptSignal::Wait
	 */
	bool Idle = false;

public:
	/**
	 * @brief Delete all connections and deconstruct Signal
	 *
	 * As connections are pointers pointing to objects, which can also be
	 * independent of the constructor scope, it is necessary to group them
	 * into an vector, and use it to deallocate all existing connections
	 * when the Signal is deconstructed
	 *
	 * @see ScriptSignal::Connections
	 */
	virtual ~ScriptSignal() {
		for (const auto& Connection : Connections) {
			delete Connection;
		}
	}

	/**
	 * @brief Create a new connection and it's function
	 *
	 * Creates a pointer to a new connection struct and push back the
	 * connection's function in ScriptSignal::Functions vector.
	 * The connection is constructed with a referenced lambda
	 * that can call ScriptSignal::Functions to after erase
	 * the current connection's function
	 *
	 * @see ScriptSignal::Connection:Connection
	 *
	 * @param Function Function or lambda to be used in connection
	 *
	 * @return ScriptSignal::Connection*
	 */
	virtual Connection* Connect(const f_(Parameters...)& Function) {
		Connection* New = new Connection([this, Index = Functions.size()] {
			Functions.erase(Functions.begin() + Index);
		});

		Functions.push_back(Function);
		Connections.push_back(New);
		return New;
	}

	/**
	 * @brief Call all functions in ScriptSignal::Functions vector
	 *
	 * If `Functions.empty()` is `false`, the function calls all
	 * ScriptSignal::Functions with the given arguments in base of
	 * the parameters created in Signal construct
	 *
	 * @note Holds mutex and set Idle to `true`, and notify all
	 * ScriptSignal::Wait waiting for ScriptSignal::Fire to be
	 * called, by ScriptSignal::Condition
	 *
	 * @see ScriptSignal::Current
	 * @see ScriptSignal::Idle
	 *
	 * @param Arguments Arguments in base of Signal's parameters
	 */
	virtual void Fire(Parameters... Arguments) {
		if (Functions.empty()) {
			return;
		}

		for (const auto& Function : Functions) {
			Function(Arguments...);
		}

		std::lock_guard<std::mutex> Hold(Current);
		Idle = true; Condition.notify_all();
	}

	/**
	 * @brief Wait for ScriptSignal::Fire to be called and return elapsed time
	 *
	 * Sets Idle to `false`, creates a new steady clock for duration, locks
	 * mutex, and waits for Idle to be `true` (that is done by ScriptSignal::Fire)
	 * and return the duration holded by the steady clock
	 *
	 * @see ScriptSignal::Current
	 * @see ScriptSignal::Idle
	 *
	 * @return long long Elapsed time to wait for ScriptSignal::Fire to be called
	 */
	long long Wait() {
		Idle = false;
		using Clock = std::chrono::steady_clock;
		Clock::time_point Time = Clock::now();
		std::unique_lock Lock(Current);
		Condition.wait(Lock, [this] { return Idle; });
		return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - Time).count();
	}
};

#undef f_
#endif

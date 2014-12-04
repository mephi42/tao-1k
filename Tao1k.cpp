#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <orbsvcs/CosNamingC.h>
#include <tao/Object.h>
#include <tao/ORB.h>
#include <thread>
#include <vector>

class Context {
public:
	Context(int argc, char** argv) : stopped(false) {
		orb = CORBA::ORB_init(argc, argv, "tao-1k");
		CORBA::Object_var namingContextObject =
			orb->resolve_initial_references("NameService");
		namingContext = CosNaming::NamingContext::_narrow(namingContextObject);
		name.length(1);
		name[0].id = CORBA::string_dup("tao-1k");
	}

	~Context() {
		namingContext = CosNaming::NamingContext::_nil();
		orb->destroy();
		orb = CORBA::ORB::_nil();
	}

	void run() {
		while (true) {
			do {
				std::unique_lock<std::mutex> g(mutex);
				if (stopped) {
					break;
				}
			} while (false);

			try {
				namingContext->resolve(name);
			}
			catch (const CosNaming::NamingContext::NotFound&) {
				continue;
			}
			catch (const CORBA::Exception& ex) {
				std::cerr << ex._info() << std::endl;
				stop(1);
				break;
			}
		}
	}

	void stop(int _rc) {
		std::unique_lock<std::mutex> g(mutex);
		if (!stopped) {
			stopped = true;
			rc = _rc;
			conditionVariable.notify_all();
		}
	}

	int wait() {
		std::unique_lock<std::mutex> g(mutex);
		while (!stopped) {
			conditionVariable.wait(g);
		}
		return rc;
	}

private:
	CORBA::ORB_var orb;
	CosNaming::NamingContext_var namingContext;
	CosNaming::Name name;
	std::mutex mutex;
	std::condition_variable conditionVariable;
	bool stopped;
	int rc;
};

int main(int argc, char** argv) {
	int nThreads = 1000;
	try {
		Context context(argc, argv);
		std::cerr << "nThreads = " << nThreads << std::endl;
		std::vector<std::unique_ptr<std::thread>> threads;
		threads.reserve(nThreads);
		auto run = std::bind(&Context::run, std::ref(context));
		for (int i = 0; i < nThreads; ++i) {
			std::unique_ptr<std::thread> thread(new std::thread(run));
			threads.push_back(std::move(thread));
		}
		int rc = context.wait();
		for (auto it = threads.begin(); it != threads.end(); ++it) {
			(*it)->join();
		}
		return rc;
	}
	catch (const CORBA::Exception& ex) {
		std::cerr << ex._info() << std::endl;
		return 1;
	}
}

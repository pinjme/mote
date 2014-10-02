
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>


using boost::uuids::uuid;
using namespace std;


class Mote {

public:
	Mote(uuid u) : _uuid(u) {


	};

	Mote(std::string& u) {

		// Convert the UUID string to an actual boost uuid:
		boost::uuids::string_generator gen;
		this->_uuid=gen(u);
	};

	Mote() {

		this->_uuid=boost::uuids::random_generator()();
	}

	uuid _uuid;
	void* data;
	unsigned int data_type;
	size_t data_len;

};
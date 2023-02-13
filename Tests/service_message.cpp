#include "service_message.hpp"

#include <gtest/gtest.h>

TEST(ServiceMessage, service_message_creation)
{
	service_message::ServiceMessage msg("sample_test_service", "_test.service.discovery", 3000);

	ASSERT_EQ(msg.name(), std::string("sample_test_service"));
	ASSERT_EQ(msg.domain(), std::string("_test.service.discovery"));
	ASSERT_EQ(msg.ttl_msec(), 3000);
}

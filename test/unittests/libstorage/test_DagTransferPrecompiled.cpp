/*
 * @CopyRight:
 * FISCO-BCOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FISCO-BCOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>
 * (c) 2016-2018 fisco-dev contributors.
 */
/** @file test_DagTransferPrecompiled.cpp
 *  @author octopuswang
 *  @date 20190111
 */
#include "Common.h"
#include "MemoryStorage.h"
#include <json_spirit/JsonSpiritHeaders.h>
#include <libblockverifier/ExecutiveContextFactory.h>
#include <libdevcrypto/Common.h>
#include <libethcore/ABI.h>
#include <libprecompiled/DagTransferPrecompiled.h>
#include <libstorage/MemoryTable.h>
#include <libstoragestate/StorageStateFactory.h>
#include <boost/test/unit_test.hpp>

using namespace dev;
using namespace dev::blockverifier;
using namespace dev::storage;
using namespace dev::storagestate;
using namespace dev::precompiled;

namespace test_DagTransferPrecompiled
{
struct DagTransferPrecompiledFixture
{
    DagTransferPrecompiledFixture()
    {
        blockInfo.hash = h256(0);
        blockInfo.number = 0;
        context = std::make_shared<ExecutiveContext>();
        ExecutiveContextFactory factory;
        auto storage = std::make_shared<MemoryStorage>();
        auto storageStateFactory = std::make_shared<StorageStateFactory>(h256(0));
        factory.setStateStorage(storage);
        factory.setStateFactory(storageStateFactory);
        factory.initExecutiveContext(blockInfo, h256(0), context);
        dtPrecompiled = std::make_shared<DagTransferPrecompiled>();
        memoryTableFactory = context->getMemoryTableFactory();
    }

    ~DagTransferPrecompiledFixture() {}

    ExecutiveContext::Ptr context;
    MemoryTableFactory::Ptr memoryTableFactory;
    DagTransferPrecompiled::Ptr dtPrecompiled;
    BlockInfo blockInfo;

    std::string userAddFunc{"userAdd(string,uint256)"};
    std::string userSaveFunc{"userSave(string,uint256)"};
    std::string userDrawFunc{"userDraw(string,uint256)"};
    std::string userTransferFunc{"userTransfer(string,string,uint256)"};
    std::string userBalanceFunc{"userBalance(string)"};
};

BOOST_FIXTURE_TEST_SUITE(test_DagTransferPrecompiled, DagTransferPrecompiledFixture)

BOOST_AUTO_TEST_CASE(toString)
{
    BOOST_TEST(dtPrecompiled->toString(context) == "DagTransfer");
}

BOOST_AUTO_TEST_CASE(isDagPrecompiled)
{
    auto ret = dtPrecompiled->isDagPrecompiled();
    BOOST_TEST(ret);
}

BOOST_AUTO_TEST_CASE(invalidUserName)
{
    std::string user0;
    BOOST_REQUIRE(user0.empty());
    bool ret = dtPrecompiled->invalidUserName(user0);
    BOOST_TEST(ret);

    std::string user1 = "user";
    BOOST_REQUIRE(!user1.empty());
    ret = dtPrecompiled->invalidUserName(user1);
    BOOST_TEST((!ret));
}

BOOST_AUTO_TEST_CASE(getDagTag)
{
    // valid user name with valid amount
    std::string user = "user";
    dev::u256 amount = 1111111;
    dev::eth::ContractABI abi;
    bytes param;

    std::vector<std::string> vTags;
    // add
    param = abi.abiIn(userAddFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(((vTags.size() == 1) && (vTags[0] == user)));
    // save
    param = abi.abiIn(userSaveFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(((vTags.size() == 1) && (vTags[0] == user)));
    // draw
    param = abi.abiIn(userDrawFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(((vTags.size() == 1) && (vTags[0] == user)));
    // balance
    param = abi.abiIn(userBalanceFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());

    // invalid user name
    user = "";
    amount = 0;
    // add
    param = abi.abiIn(userAddFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());
    // save
    param = abi.abiIn(userSaveFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());
    // draw
    param = abi.abiIn(userDrawFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());
    // balance
    param = abi.abiIn(userBalanceFunc, user, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());

    // transfer test
    // valid input parameters
    std::string from = "from";
    std::string to = "to";
    amount = 1111111;
    param = abi.abiIn(userTransferFunc, from, to, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(((vTags.size() == 2) && (vTags[0] == from) && (vTags[1] == to)));

    // from user empty
    from = "";
    to = "to";
    amount = 1111111;
    param = abi.abiIn(userTransferFunc, from, to, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());

    // to user empty
    from = "from";
    to = "";
    amount = 1111111;
    param = abi.abiIn(userTransferFunc, from, to, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());

    // amount zero
    from = "from";
    to = "to";
    amount = 0;
    param = abi.abiIn(userTransferFunc, from, to, amount);
    vTags = dtPrecompiled->getDagTag(bytesConstRef(&param));
    BOOST_TEST(vTags.empty());
}

BOOST_AUTO_TEST_CASE(userAdd)
{  // function userAdd(string user, uint256 balance) public returns(bool);
    Address origin;
    dev::eth::ContractABI abi;

    std::string user;
    dev::u256 amount;
    bytes out;
    bool result;
    bytes params;

    // invalid input, user name empty string
    user = "";
    params = abi.abiIn(userAddFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&params), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST((!result));

    // normal input, first add this user
    user = "user";
    amount = 11111;
    params = abi.abiIn(userAddFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&params), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    // user already exist, add this user again
    user = "user";
    amount = 11111;
    params = abi.abiIn(userAddFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&params), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));
}

BOOST_AUTO_TEST_CASE(userSave)
{  // function userSave(string user, uint256 balance) public returns(bool);
    Address origin;
    dev::eth::ContractABI abi;

    std::string user;
    dev::u256 amount;
    bytes out;
    bool result;
    bytes params;

    // invalid input, user name empty string
    user = "";
    params = abi.abiIn(userSaveFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST((!result));

    // invalid input, amount zero
    user = "user";
    amount = 0;
    params = abi.abiIn(userSaveFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST((!result));

    // normal input, user is not exist, add this user.
    user = "user";
    amount = 1111;
    params = abi.abiIn(userSaveFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    // normal input, user exist, add balance of this user.
    user = "user";
    amount = 1111;
    params = abi.abiIn(userSaveFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(out, result);
    BOOST_TEST(result);
    // get balance of this user
    dev::u256 balance;
    params = abi.abiIn(userBalanceFunc, user);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(((result) && (balance == (amount + amount))));

    // normal input, user exist, add balance of this user, balance overflow.
    user = "user";
    amount = dev::u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    params = abi.abiIn(userSaveFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));
}

BOOST_AUTO_TEST_CASE(userDraw)
{  // function userDraw(string user, uint256 balance) public returns(bool);
    Address origin;
    dev::eth::ContractABI abi;

    std::string user;
    dev::u256 amount = 0;
    bytes out;
    bool result;
    bytes params;

    // invalid input, user name empty string
    user = "";
    params = abi.abiIn(userDrawFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST((!result));

    // invalid input, amount zero
    user = "user";
    amount = 0;
    params = abi.abiIn(userDrawFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST((!result));

    // add user first, after this the balance of "user" is 11111
    user = "user";
    amount = 11111;
    params = abi.abiIn(userAddFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    // draw 11110 , after this the balance of "user" is 1
    user = "user";
    amount = 11110;
    params = abi.abiIn(userDrawFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    // draw 11110  again, insufficient balance
    params = abi.abiIn(userDrawFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));

    // get balance of this user
    dev::u256 balance;
    params = abi.abiIn(userBalanceFunc, user);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(((result) && (balance == dev::u256(1))));
}

BOOST_AUTO_TEST_CASE(userBalance)
{  // function userBalance(string user) public constant returns(bool,uint256);
    Address origin;
    dev::eth::ContractABI abi;

    std::string user;
    dev::u256 balance;
    bool result;
    bytes out;
    bytes params;

    // invalid input, user name empty string
    user = "";
    balance = 0;
    params = abi.abiIn(userBalanceFunc, user);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(!(result));

    // normal input, user not exist
    user = "user";
    balance = 0;
    params = abi.abiIn(userBalanceFunc, user);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(!(result));

    // create user
    user = "user";
    amount = 1111111;
    params = abi.abiIn(userAddFunc, user, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(result);

    // get balance of user
    params = abi.abiIn(userBalanceFunc, user);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(((result) && (balance == amount))) d;
}

BOOST_AUTO_TEST_CASE(userTransfer)
{  // function userTransfer(string user_a, string user_b, uint256 amount) public returns(bool);
    Address origin;
    dev::eth::ContractABI abi;

    std::string from, to;
    dev::u256 amount;
    bool result;
    bytes out;
    bytes params;

    // invalid input, from user name empty string, to user name empty string.
    params = abi.abiIn(userTransferFunc, from, to, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));

    // invalid input, from user name empty string.
    from = "";
    to = "to";
    amount = 12345;
    params = abi.abiIn(userTransferFunc, from, to, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(out, result);
    BOOST_TEST(!(result));

    // invalid input, to user name empty string.
    from = "from";
    to = "";
    amount = 12345;
    params = abi.abiIn(userTransferFunc, from, to, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));

    // invalid input, amount zero.
    from = "from";
    to = "to";
    amount = 0;
    params = abi.abiIn(userTransferFunc, from, to, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));

    // from and to user all not exist
    from = "from";
    to = "to";
    amount = 11111;
    params = abi.abiIn(userTransferFunc, from, to, amount);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));

    // insert three user: user0(111111)  user1(2222222)
    // user3(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    std::string user0 = "user0";
    dev::u256 amount0 = 111111;
    params = abi.abiIn(userAddFunc, user0, amount0);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    std::string user1 = "user1";
    dev::u256 amount1 = 2222222;
    params = abi.abiIn(userAddFunc, user1, amount1);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    std::string user2 = "user2";
    dev::u256 amount2("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    params = abi.abiIn(userAddFunc, user2, amount2);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    // user0 transfer 111110 to user1
    dev::u256 transfer = 111110;
    params = abi.abiIn(userTransferFunc, user0, user1, transfer);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(result);

    // user0 transfer 111110 to user1 again
    dev::u256 transfer = 111110;
    params = abi.abiIn(userTransferFunc, user0, user1, transfer);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));

    dev::u256 balance;
    // get balance of user0
    params = abi.abiIn(userBalanceFunc, user0);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(((result) && (balance == (amount0 - transfer)));

    params = abi.abiIn(userBalanceFunc, user1);
    // get balance of user1
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result, balance);
    BOOST_TEST(((result) && (balance == (amount1 + transfer)));

    // user1 transfer 111110 to user2, balance of user2 will overflow
    params = abi.abiIn(userTransferFunc, user1, user2, transfer);
    out = dtPrecompiled->call(context, bytesConstRef(&param), origin);
    abi.abiOut(bytesConstRef(&out), result);
    BOOST_TEST(!(result));
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace test_DagTransferPrecompiled
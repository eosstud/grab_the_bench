#include "grab_the_bench.hpp"

uint64_t price_inc(uint64_t price) {

	// price increment
	return 1;
}

void grab_the_bench::withdraw( const account_name account, asset quantity ) {

	// find user
	auto itr = _balance.find( account );
	eosio_assert( itr != _balance.end(), "user does not exist" );

	// set quantity
	eosio_assert( quantity.amount + itr->balance > quantity.amount, "integer overflow adding withdraw balance" );
	quantity.amount += itr->balance;

	// clear balance
	_balance.modify(itr, _this_contract, [&](auto& p) {
		p.balance = 0;
	});

	// withdraw
	action(
		permission_level{ _this_contract, N(active) },
		N(eosio.token), N(transfer),
		std::make_tuple( _this_contract, account, quantity, std::string("") )
	).send();
}

void grab_the_bench::on( const currency::transfer& t, account_name code ) {

	// ping
	ping();

	// return if transfer is from current contract
	if (t.from == _this_contract)
		return;

	// transfer must be EOS token from eosio.token
	eosio_assert(code == N(eosio.token), "transfer not from eosio.token");
	eosio_assert(t.to == _this_contract, "transfer not made to this contract");

	// if transfer amount is 0.0001 EOS then withdraw
	if (t.quantity.amount == 1) {
		withdraw( t.from, t.quantity );
		return;
	}

	// quantity check
	// TODO eosio_assert(t.quantity.symbol == asset().symbol, "asset must be system token");
	eosio_assert(t.quantity.amount >= MINIMUM, "quantity must be greater than minimum amount");
	eosio_assert(t.quantity.is_valid(), "invalid quantity");

	auto user = t.from;
	auto amount = t.quantity.amount;

	// retrieve counter and price
	auto counter = _counters.begin();
	auto price = counter->key_price;

	// pay dividend
	uint64_t dividend = _players.begin() == _players.end() ? 0 : ( amount * DIVIDEND_SIZE ) ;
	for (auto itr = _players.begin(); itr != _players.end(); itr++) {

		auto share = dividend * ( itr->keys / counter->sold_keys );
		auto useritr = _balance.find( itr->player_name );

		if ( useritr == _balance.end() ) {
			useritr = _balance.emplace( _this_contract, [&](auto& p){
				p.username = itr->player_name;
			});
		}

		_balance.modify( useritr, _this_contract, [&](auto& p){
			eosio_assert( p.total + share > p.total, "integer overflow adding user balance total" );
			p.total += share;
			eosio_assert( p.balance + share > p.balance, "integer overflow adding user balance balance" );
			p.balance += share;
		});
	}

	// pay team reward
	uint64_t team_reward = amount * TEAM_REWARD_SIZE;
	auto team = _balance.find( team_account );
	if ( team == _balance.end() ) {
		team = _balance.emplace( _this_contract, [&](auto& p) {
			p.username = team_account;
		});
	}
	_balance.modify( team, _this_contract, [&](auto& p){
		eosio_assert( p.total + team_reward > p.total, "integer overflow adding team reward total" );
		p.total += team_reward;
		eosio_assert( p.balance + team_reward > p.balance, "integer overflow adding team reward balance" );
		p.balance += team_reward;
	});

	// find user
	auto itr = _players.find( user );
	if ( itr == _players.end() ) {
		itr = _players.emplace( _this_contract, [&](auto& p) {
			p.player_name = user;
		});
	}

	// new keys and price
	double new_keys = 0;
	while ( amount > price ) {
		new_keys++;

		eosio_assert( amount - price < amount, "integer underflow subtracting purchase amount" );
		amount -= price;

		eosio_assert( price + price_inc( price ) > price, "integer overflow adding price increment" );
		price += price_inc( price );
	}

	new_keys += (double)amount / (double)price;

	eosio_assert( price + price_inc( price ) > price, "integer overflow adding price increment" );
	price += price_inc( price );

	// update tables
	_players.modify(itr, _this_contract, [&](auto& p) {
		eosio_assert( p.keys + new_keys > p.keys, "integer overflow adding new keys" );
		p.keys += new_keys;
		eosio_assert( p.amount + t.quantity.amount > p.amount, "integer overflow adding player amount" );
		p.amount += t.quantity.amount;
	});

	_counters.modify(counter, _this_contract, [&](auto& p){

		// purchase a whole new key to win
		if ( new_keys >= 1 ) {
			p.last_buyer = user;
			p.last_buy_time = now();
			p.end_time = std::min( p.end_time + TIME_INC, p.last_buy_time + MAX_TIME_INC );
		}

		eosio_assert( p.balance + t.quantity.amount > p.balance, "integer overflow adding counter balance" );
		p.balance += t.quantity.amount;

		eosio_assert( p.pot + t.quantity.amount - dividend - team_reward > p.pot, "integer overflow adding pot" );
		p.pot += t.quantity.amount - dividend - team_reward;

		eosio_assert( p.sold_keys + new_keys > p.sold_keys, "integer overflow adding sold_keys" );
		p.sold_keys += new_keys;

		p.key_price = price;
	});
}

void grab_the_bench::ping() {

	auto counter = _counters.begin();

	// game ends
	if ( counter->end_time <= now() ) {

		// get winner account
		auto winner = counter->last_buyer;

		// find winner entry in _balance
		auto witr = _balance.find(winner);
		if ( witr == _balance.end() ) {
			witr = _balance.emplace( _this_contract, [&](auto& p){
				p.username = winner;
			});
		}

		// update winner entry in _balance
		_balance.modify( witr, _this_contract, [&](auto& p){
			eosio_assert( p.total + counter->pot > p.total, "integer overflow adding winner total" );
			p.total += counter->pot;
			eosio_assert( p.balance + counter->pot > p.balance, "integer overflow adding winner balance" );
			p.balance += counter->pot;
		});

		// erase tables
		for (auto itr = _counters.begin(); itr != _counters.end();)
			itr = _counters.erase(itr);
		for (auto itr = _players.begin(); itr != _players.end();)
			itr = _players.erase(itr);

		// start new game
		_counters.emplace( _this_contract, [&](auto& p) {
			p.owner = _this_contract;
			p.last_buyer = _this_contract;
		});
	}
}

void grab_the_bench::hi( account_name user ) {

	// user auth
	require_auth(user);
}

void grab_the_bench::erase() {

	// user auth
	require_auth(_this_contract);

	// erase tables
	for (auto itr = _counters.begin(); itr != _counters.end();)
		itr = _counters.erase(itr);
	for (auto itr = _players.begin(); itr != _players.end();)
		itr = _players.erase(itr);
	for (auto itr = _balance.begin(); itr != _balance.end();)
		itr = _balance.erase(itr);
}

void grab_the_bench::maintain() {

	// user auth
	require_auth(_this_contract);
}

void grab_the_bench::apply( account_name contract, account_name act ) {

	if ( act == N(transfer) ) {
		on( unpack_action_data<currency::transfer>(), contract );
		return;
	}

	if ( contract != _this_contract )
		return;

	auto& thiscontract = *this;
	switch( act ) {
		EOSIO_API( grab_the_bench, (ping)(hi)(erase)(maintain) );
	};
}

extern "C" {
	[[noreturn]] void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
		grab_the_bench gtb( receiver );
		gtb.apply( code, action );
		eosio_exit(0);
	}
}

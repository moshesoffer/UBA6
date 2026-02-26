const { withTimeout, AWAIT_TIMEOUT } = require('../utils/requestSync');

const username = 'amicell';
const password = '1q!QazAZ';

exports.login = async (req, res) => {
	const displayName = {
		name: 'Natasha Cherkover'
	};

	await withTimeout(new Promise(resolve), AWAIT_TIMEOUT);

	if (req.body?.username === username && req.body?.password === password) {
		res.json(displayName);

		return;
	}

	res
		.status(400)
		.send('Invalid credentials');
};

exports.logout = async (req, res) => {
	await withTimeout(new Promise(resolve), AWAIT_TIMEOUT);

	res.end();
};


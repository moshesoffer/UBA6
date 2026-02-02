const username = 'amicell';
const password = '1q!QazAZ';

exports.login = async (req, res) => {
	const displayName = {
		name: 'Natasha Cherkover'
	};

	await new Promise(resolve => setTimeout(() => resolve(), 2000));

	if (req.body?.username === username && req.body?.password === password) {
		res.json(displayName);

		return;
	}

	res
		.status(400)
		.send('Invalid credentials');
};

exports.logout = async (req, res) => {
	await new Promise(resolve => setTimeout(() => resolve(), 2000));
	res.end();
};


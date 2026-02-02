const reactRecommended = require( 'eslint-plugin-react/configs/recommended' );
const reactHooks = require( 'eslint-plugin-react-hooks' );
const js = require( '@eslint/js' );

module.exports = [
	reactRecommended,
	js.configs.recommended,
	// js.configs.all, // Uncomment this line to enable all recommended rules
	{
		plugins: {
			'react-hooks': reactHooks,
		},
		rules: {
			...reactHooks.configs.recommended.rules,
		},
		ignores: [
			'**/dist/*',
			'**/public/*',
			'**/out/*',
			'**/node_modules/*',

			'**/.next/*',
			'next.config.js',

			'vite.config.js',
			'vite.config.ts',

			'src/reportWebVitals.js',
			'src/service-worker.js',
			'src/serviceWorkerRegistration.js',
			'src/setupTests.js',

			'src/reportWebVitals.ts',
			'src/service-worker.ts',
			'src/serviceWorkerRegistration.ts',
			'src/setupTests.ts',
		],
		settings: {
			react: {
				version: 'detect',
			}
		}
	},
];

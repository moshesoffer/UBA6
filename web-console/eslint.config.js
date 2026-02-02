const globals = require('globals');
const reactRecommended = require('eslint-plugin-react/configs/recommended');
const reactHooks = require('eslint-plugin-react-hooks');
const js = require('@eslint/js');

module.exports = [
	reactRecommended,
	// js.configs.recommended,
	js.configs.all,
	{
		languageOptions: {
			ecmaVersion: 2022,
			sourceType: 'module',
			globals: {
				...globals.browser,
			}
		},
		plugins: {
			'react-hooks': reactHooks,
		},
		rules: {
			...reactHooks.configs.recommended.rules,
			// js.configs.all rules customizations.
			'one-var': 'off',
			'sort-keys': 'off',
			'sort-imports': 'off',
			'no-magic-numbers': 'off',
			'max-statements': 'off',
			'max-params': 'off',
			'max-lines-per-function': 'off',
			'max-lines': 'off',
			'no-bitwise': 'off',
			'complexity': 'off',
			'no-undef': 'warn',
			'no-unused-vars': 'warn',
			'func-style': 'warn',
			'arrow-body-style': 'warn',
			'no-console': 'warn',
			'no-shadow': 'warn',
			'no-use-before-define': 'warn',
			'capitalized-comments': 'warn',
			'no-underscore-dangle': 'warn',
			'prefer-promise-reject-errors': 'warn',
			'consistent-return': 'warn',
			'require-await': 'error',
			'prefer-const': 'warn',
			'no-inline-comments': 'warn',
			'object-shorthand': 'warn',
			'no-implicit-coercion': 'warn',
			'id-length': 'warn',
			'prefer-regex-literals': 'warn',
			'require-unicode-regexp': 'warn',
			'no-ternary': 'warn',
			'init-declarations': 'warn',
			'curly': 'warn',
			'no-param-reassign': 'warn',
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

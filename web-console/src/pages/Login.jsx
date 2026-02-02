import {useState,} from 'react';

import Box from '@mui/material/Box';
import Link from '@mui/material/Link';
import Card from '@mui/material/Card';
import Stack from '@mui/material/Stack';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import IconButton from '@mui/material/IconButton';
import {alpha, useTheme} from '@mui/material/styles';
import InputAdornment from '@mui/material/InputAdornment';
import Button from '@mui/material/Button';

import {useAuthDispatch,} from 'src/store/AuthProvider';
import {login,} from 'src/action-creators/Auth';
import {bgGradient,} from 'src/theme/css';
import Logo from 'src/components/logo';
import Iconify from 'src/components/Iconify';
import {getText,} from 'src/services/string-definitions';

export default function Login() {

	const theme = useTheme();
	const authDispatch = useAuthDispatch();

	const [username, setUsername] = useState('');
	const [password, setPassword] = useState('');
	const [showPassword, setShowPassword] = useState(false);

	const handleClick = event => {
		event.preventDefault();

		login(authDispatch, {
			username: username.trim(),
			password: password.trim(),
		});
	}

	return (
		<Box sx={{
			...bgGradient({
				color: alpha(theme.palette.background.default, 0.9),
				imgUrl: '/assets/background/overlay_4.jpg',
			}),
			height: 1,
		}}>
			<Logo sx={{
				position: 'fixed',
				top: {xs: 16, md: 24},
				left: {xs: 16, md: 24},
			}}/>

			<Stack alignItems="center" justifyContent="center" sx={{height: 1}}>
				<Card sx={{p: 5, width: 1, maxWidth: 420,}}>
					<Typography variant="h4" sx={{mb: 5}}>
						{getText('pages.WELCOME')}
					</Typography>

					<Stack spacing={3}>
						<TextField name="email" label={getText('pages.EMAIL_ADDRESS')} onChange={event => setUsername(event.target.value)}/>

						<TextField
							name="password"
							label={getText('pages.PASSWORD')}
							type={showPassword ? 'text' : 'password'}
							InputProps={{
								endAdornment: (
									<InputAdornment position="end">
										<IconButton onClick={() => setShowPassword(!showPassword)} edge="end">
											<Iconify icon={showPassword ? 'eva:eye-fill' : 'eva:eye-off-fill'}/>
										</IconButton>
									</InputAdornment>
								),
							}}
							onChange={event => setPassword(event.target.value)}
						/>
					</Stack>

					<Stack direction="row" alignItems="center" justifyContent="flex-end" sx={{my: 3}}>
						<Link variant="subtitle2" underline="hover">
							{getText('pages.FORGOT_PASSWORD')}
						</Link>
					</Stack>

					<Button fullWidth size="large" type="submit" variant="contained" color="inherit" onClick={handleClick}>
						{getText('pages.LOGIN')}
					</Button>
				</Card>
			</Stack>
		</Box>
	);
}

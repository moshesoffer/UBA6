import {useEffect,} from 'react';
import {Outlet, useRoutes, useLocation,} from 'react-router-dom';

import Layout from 'src/layout';
import RequireAuth from 'src/routes/RequireAuth';
import RequirePublic from 'src/routes/RequirePublic';

import MainPage from 'src/pages/main-page';
import TestEditor from 'src/pages/test-routines';
import ReportsPage from 'src/pages/reports-page';
import SettingsPage from 'src/pages/settings-page';
import UsersPage from 'src/pages/users-page';
import NotFound from 'src/pages/NotFound';
import Login from 'src/pages/Login';
import {useUbaDevicesDispatch} from 'src/store/UbaDevicesProvider';
import ReportsProvider from 'src/store/ReportsProvider';
import {navigationPaths,} from 'src/constants/unsystematic';
import {setCurrentUba, setSelectedDevices,} from 'src/actions/UbaDevices';

export default function Router() {

	const {pathname,} = useLocation();
	const ubaDevicesDispatch = useUbaDevicesDispatch();

	useEffect(() => {
		ubaDevicesDispatch(setCurrentUba({}));
		ubaDevicesDispatch(setSelectedDevices([]));
	}, [pathname,]);

	return useRoutes([
		{
			element: (
				<Layout>
					<Outlet/>
				</Layout>
			),
			children: [
				{
					index: true,
					element: (
						<RequireAuth>
							<MainPage/>
						</RequireAuth>
					),
				},
				{
					path: navigationPaths.TEST_ROUTINES,
					element: (
						<RequireAuth>
							<TestEditor/>
						</RequireAuth>
					),
				},
				{
					path: navigationPaths.REPORTS,
					element: (
						<RequireAuth>
							<ReportsProvider>
								<ReportsPage/>
							</ReportsProvider>
						</RequireAuth>
					),
				},
				{
					path: navigationPaths.SETTINGS,
					element: (
						<RequireAuth>
							<SettingsPage/>
						</RequireAuth>
					),
				},
				{
					path: navigationPaths.USERS,
					element: (
						<RequireAuth>
							<UsersPage/>
						</RequireAuth>
					),
				},
			],
		},
		{
			path: 'login',
			element: (
				<RequirePublic>
					<Login/>
				</RequirePublic>
			),
		},
		{
			path: '*',
			element: <NotFound/>,
		},
	]);
}

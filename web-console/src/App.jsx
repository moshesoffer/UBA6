import { BrowserRouter } from 'react-router-dom';

import Router from 'src/routes/Router';
import ThemeProvider from 'src/theme';

import AuthProvider from 'src/store/AuthProvider';
import SettingsProvider from 'src/store/SettingsProvider';
import TestRoutinesProvider from 'src/store/TestRoutinesProvider';
import UbaDevicesProvider from 'src/store/UbaDevicesProvider';

import { defaults } from "chart.js/auto";
import LoadingScreen from 'src/components/LoadingScreen';
import Notifications from 'src/components/Notifications';

import {
	CategoryScale,
	Chart as ChartJS,
	Legend,
	LinearScale,
	LineElement,
	PointElement,
	Title,
	Tooltip
} from "chart.js";
import zoomPlugin from "chartjs-plugin-zoom";

defaults.plugins.title.display = true;
defaults.plugins.title.font.size = 20;
defaults.plugins.title.color = "black";

ChartJS.register(
	CategoryScale,
	LinearScale,
	PointElement,
	LineElement,
	Title,
	Tooltip,
	Legend,
	zoomPlugin
  );
  
export default function App() {

	return (
		<BrowserRouter>
			<ThemeProvider>
				<AuthProvider>
					<UbaDevicesProvider>
						<TestRoutinesProvider>
							<SettingsProvider>
								<LoadingScreen/>
								<Notifications/>
								<Router/>
							</SettingsProvider>
						</TestRoutinesProvider>
					</UbaDevicesProvider>
				</AuthProvider>
			</ThemeProvider>
		</BrowserRouter>
	);
}

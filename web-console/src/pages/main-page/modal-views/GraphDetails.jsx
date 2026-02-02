import Box from '@mui/material/Box';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import { useEffect, useState } from 'react';
import { Line } from 'react-chartjs-2';
import withModalView from 'src/components/withModalView';
import { getKeyByValue, statusCodes} from 'src/constants/unsystematic';
import { getText, } from 'src/services/string-definitions';
import { useTestRoutines, useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import { useUbaDevices, } from 'src/store/UbaDevicesProvider';
import { validateArray } from 'src/utils/validators';
import { setGraphData,} from 'src/actions/TestRoutines';

const zoomOptions = {
	pan: {
		enabled: true,
		mode: 'xy',

	},
	zoom: {
		wheel: {
			enabled: true
		},
		pinch: {
			enabled: true
		},
		mode: 'xy',
	}
};

const defaultOptions = {
	responsive: true,
	spanGaps: true,
	scales:{
		x: {
			position: 'bottom',
		},
		y: {
			position: 'left',
			title: {
				color: 'green',
				display: true,
				text: 'Volts [V]'
			},
			ticks: {
				color: 'green',
			}
		},
		y1: {
			position: 'right',
			title: {
				color: 'blue',
				display: true,
				text: 'Current [mA]',
			},
			grid: {
			  drawOnChartArea: false, // only want the grid lines for one axis to show up
			},
			ticks: {
				color: 'blue',
			}
		},
	},
	plugins: {
	  legend: {
		position: "bottom"
	  },
	  tooltip: {
		usePointStyle: true,
	  },
	  title: {
		display: true,
		text: "Live Test Results Graph - Volt / Current / Temperature vs. Time",
	  },
	  zoom: zoomOptions
	}
};

function GraphDetails() {

	const {currentUba,} = useUbaDevices();
	const {graphData,} = useTestRoutines();
	const lastReading = graphData.at(-1);
	const [lineDataSets, setLineDataSets] = useState([]);
	const [graphsInfo, setGraphsInfo] = useState([]);

	const [options, setOptions] = useState(defaultOptions);
	const testRoutinesDispatch = useTestRoutinesDispatch();

	useEffect(() => {
		//console.log('GraphDetails mounted');
		return () => {
			//console.log('GraphDetails un mounted');
			setLineDataSets([]);
			setGraphsInfo([]);
			setOptions(defaultOptions);
			testRoutinesDispatch(setGraphData([]));
		};
	}, []);

	useEffect(() => {
		if(validateArray(graphData)) {
			const lds = prepareLineDataSets(graphData);
			const graphInfoArr = prepareGraphsInfoArr(graphData, lds);

			if(graphInfoArr.some(item => item.yAxisID === 'y2')) {
				//meaning that there is temperature data
				const copiedObject = JSON.parse(JSON.stringify(defaultOptions));
				copiedObject.scales.y2 = {
					position: 'right',
					title: {
						color: 'red',
						display: true,
						text: 'Temperature [℃]',
					},
					grid: {
					  drawOnChartArea: false, // only want the grid lines for one axis to show up
					},
					ticks: {
						color: 'red',
					}
				};
				setOptions(copiedObject);
			}

			setLineDataSets(lds);
			setGraphsInfo(graphInfoArr);
		} else {
			setLineDataSets([]);
			setGraphsInfo([]);
			setOptions(defaultOptions);
		}
	}, [graphData]);

	const prepareGraphsInfoArr = (arr, lds) =>{
		const graphInfoArr = [];
		const temperatureGraphObj = prepareGraphsInfoObj(lds, 'temp', 'y2', 'red');
		if(temperatureGraphObj) graphInfoArr.push(temperatureGraphObj);
		const currentGraphObj = prepareGraphsInfoObj(lds, 'current', 'y1', 'blue');
		if(currentGraphObj) graphInfoArr.push(currentGraphObj);
		const voltageGraphObj = prepareGraphsInfoObj(lds, 'voltage', 'y', 'green');
		if(voltageGraphObj) graphInfoArr.push(voltageGraphObj);
		return graphInfoArr;
	}

	const prepareGraphsInfoObj = (lds, graphType, yAxisID, color) => {
		const dataFromLds = lds.map((data) => data[`${graphType}`]);
		if(dataFromLds.every(item => item === undefined)) return undefined;
		return {
			data: dataFromLds,
			label: `${graphType}`,
			borderWidth: 1,
			yAxisID: yAxisID,
			pointStyle: false,
			borderColor: color,
      		backgroundColor: color,
		};
	}

	const prepareLineDataSets = (arr) => {
		arr.sort((a, b) => a.dateTimeValue - b.dateTimeValue);
		
		return arr.map(obj => {//remove timestamp and dateTimeValue from the object
			const { timestamp, dateTimeValue, ...rest } = obj;
			return rest;
		});
	}

	return (
		<>
			<Stack direction="row" justifyContent="space-between">
				<Stack>
					<Box>
						{currentUba.ubaSN} / {currentUba.machineName} - {currentUba.testName}
					</Box>

					<Box>
						{getText('common.STATUS')}: {getKeyByValue(statusCodes, currentUba.status)}
					</Box>

					<Box>
						{getText('mainPage.LAST_READING')}: <span style={{color: "green"}}>{lastReading?.voltage}V</span> / <span style={{color: "blue"}}>{lastReading?.current}A</span> / <span style={{color: "red"}}>{lastReading?.temp ? lastReading?.temp + '℃' : ''}</span>
					</Box>
				</Stack>
			</Stack>

			{(!validateArray(lineDataSets) || !validateArray(graphsInfo)) && (
				<Typography level="title-lg">
					{
						getText('reportsPage.LOADING')//NO_DATA
					}
					<img style={{marginLeft: '10px'}}  src="/assets/images/ajax-loader.gif" alt="ajax-loader"/>
					
				</Typography>
			)}

			{validateArray(lineDataSets) && validateArray(graphsInfo) && (
				<Line
					data={{
						labels: lineDataSets.map((data) => data.timePart),
						datasets: graphsInfo,
					}}
					options={options}
				/>
			)}
		</>
	);
}

export default withModalView(GraphDetails);

import { useEffect, useState } from 'react';

import Typography from '@mui/material/Typography';
import { Line } from 'react-chartjs-2';
import withModalView from 'src/components/withModalView';
import { getText, } from 'src/services/string-definitions';
import { validateArray } from 'src/utils/validators';
import { convertTimestampToTime } from '../../main-page/utils';
import {setTestsGraph,} from 'src/actions/Reports';
import { useReports, useReportsDispatch, } from 'src/store/ReportsProvider';

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
			position: 'bottom',//center - not mandatory
		},
		y: {
			position: 'left',
			//type: 'linear',//not mandatory
			title: {
				color: 'black',
				display: true,
				text: 'Volts [V]',
			}
		},
		y1: {
			position: 'right',
			//type: 'linear',
			title: {
				color: 'black',
				display: true,
				text: 'Current [A]',//TODO!!! in future change mA its just for the show now
			},
			grid: {
			  drawOnChartArea: false, // only want the grid lines for one axis to show up
			},
		},
	},
	plugins: {
	  legend: {
		position: "bottom"
	  },
	  title: {
		display: true,
		text: "Final Test Results Graph - Volt / Current / Temperature vs. Time",
	  },
	  zoom: zoomOptions
	}
};

function ResultsGraphs() {

	const {testsGraph, reports} = useReports();
	const [lineDataSets, setLineDataSets] = useState([]);
	const [graphsInfo, setGraphsInfo] = useState([]);
	const [options, setOptions] = useState(defaultOptions);
	const reportsDispatch = useReportsDispatch();

	useEffect(() => {
		//console.log('ResultsGraphs mounted');
		return () => {
			//console.log('ResultsGraphs un mounted');
			setLineDataSets([]);
			setGraphsInfo([]);
			setOptions(defaultOptions);
			reportsDispatch(setTestsGraph([]));
		};
	}, []);

	useEffect(() => {
		if(validateArray(testsGraph)) {
			//console.log('useEffect 1', reports, testsGraph);
			const lds = prepareLineDataSets(testsGraph);
			const graphInfoArr = prepareGraphsInfoArr(testsGraph, lds);

			//console.log('lds', lds);
			//console.log('graphInfoArr', graphInfoArr);
			if(graphInfoArr.some(item => item.yAxisID === 'y2')) {
				//meaning that there is temperature data
				const copiedObject = JSON.parse(JSON.stringify(defaultOptions));
				copiedObject.scales.y2 = {
					position: 'right',
					title: {
						color: 'black',
						display: true,
						text: 'Temperature [℃]',
					},
					grid: {
					  drawOnChartArea: false, // only want the grid lines for one axis to show up
					},
				};
				setOptions(copiedObject);
			}
			
			setLineDataSets(lds);
			setGraphsInfo(graphInfoArr);
		} else {
			//console.log('useEffect 2');
			setLineDataSets([]);
			setGraphsInfo([]);
			setOptions(defaultOptions);
		}
	}, [testsGraph]);

	const prepareGraphsInfoArr = (arr, lds) =>{
		const graphInfoArr = [];
		arr.forEach((report, index) => {
			const reportDetails = reports.rows.find(obj => obj.id === report.reportID);
			const currentGraphObj = prepareGraphsInfoObj(reportDetails, index, lds, 'current', 'y1');
			if(currentGraphObj) graphInfoArr.push(currentGraphObj);
			const voltageGraphObj = prepareGraphsInfoObj(reportDetails, index, lds, 'voltage', 'y');
			if(voltageGraphObj) graphInfoArr.push(voltageGraphObj);
			const temperatureGraphObj = prepareGraphsInfoObj(reportDetails, index, lds, 'temperature', 'y2');
			if(temperatureGraphObj) graphInfoArr.push(temperatureGraphObj);
		});
		return graphInfoArr;
	}

	const prepareGraphsInfoObj = (reportDetails, index, lds, graphType, yAxisID) => {
		const dataFromLds = lds.map((data) => data[`${graphType}_${index}`]);
		if(dataFromLds.every(item => item === undefined)) return undefined;
		return {
			data: dataFromLds,
			label: `${graphType} - ${reportDetails.testName} - ${reportDetails.ubaSN}`,
			borderWidth: 1,
			pointStyle: false,
			yAxisID: yAxisID,
		};
	}

	const prepareLineDataSets = (arr) => {
		const result = [];
		
		arr.forEach((report, index) => {
		  report.testResults.forEach(dataPoint => {
			let convertedTS = convertTimestampToTime(dataPoint.timestamp);
			const existingEntry = result.find(entry => entry.timestamp === dataPoint.timestamp);
			
			if (existingEntry) {
				if(dataPoint.voltage) existingEntry[`voltage_${index}`] = dataPoint.voltage;
				if(dataPoint.current) existingEntry[`current_${index}`] = dataPoint.current;
				if(dataPoint.temperature) existingEntry[`temperature_${index}`] = dataPoint.temperature;
			} else {
			  const newEntry = {
				timestamp: dataPoint.timestamp,
				convertedTS: convertedTS
			  };
			  if(dataPoint.voltage) newEntry[`voltage_${index}`] = dataPoint.voltage;
			  if(dataPoint.current) newEntry[`current_${index}`] = dataPoint.current;
			  if(dataPoint.temperature) newEntry[`temperature_${index}`] = dataPoint.temperature;
			  result.push(newEntry);
			}
		  });
		});
	  
		result.sort((a, b) => a.timestamp - b.timestamp);
		return result.map(obj => {//remove timestamp from the object
			const { timestamp, ...rest } = obj;
			return rest;
		});
	  }
	
	
	return (
		<>
			{(!validateArray(lineDataSets) || !validateArray(graphsInfo)) && (
				<Typography level="title-lg" style={{ position: 'absolute', left: '50%', bottom: '50%'}}>
					{
						getText('reportsPage.LOADING')//NO_DATA
					}
					<img style={{marginLeft: '10px'}}  src="/assets/images/ajax-loader.gif" alt="ajax-loader"/>
				</Typography>
			)}

			{validateArray(lineDataSets) && validateArray(graphsInfo) && validateArray(testsGraph) && (
				<Line
					data={{
						labels: lineDataSets.map((data) => data.convertedTS),
						datasets: graphsInfo,
					}}
					options={options}
				/>
			)}
		</>
	);
}

export default withModalView(ResultsGraphs);

import React, { useEffect, useState } from 'react';
import PropTypes from 'prop-types';
import Tooltip from '@mui/material/Tooltip';
import GridViewIcon from '@mui/icons-material/GridView';
import IconButton from '@mui/material/IconButton';
import Toolbar from '@mui/material/Toolbar';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import { useUbaDevices, useUbaDevicesDispatch, } from 'src/store/UbaDevicesProvider';
import {getText,} from 'src/services/string-definitions';
import { setSelectedDevices, setState } from 'src/actions/UbaDevices';
import { category, pageStateList, getSelectedCardsOrTableView, setSelectedCardsOrTableView } from 'src/constants/unsystematic';
import TableViewIcon from '@mui/icons-material/TableView';

export default function CustomToolbar() {

	const {ubaTotal} = useUbaDevices();
	const ubaDevicesDispatch = useUbaDevicesDispatch();
	const [cardsOrTableView, setCardsOrTableView] = useState(pageStateList.CARDS_VIEW);

	useEffect(() => {
		const savedState = getSelectedCardsOrTableView();
		setCardsOrTableView(savedState);
		ubaDevicesDispatch(setState(savedState));
	}, []);

	const handleToggle = () => {
		const newState = cardsOrTableView === pageStateList.CARDS_VIEW ? pageStateList.TABLE_VIEW : pageStateList.CARDS_VIEW;
		setCardsOrTableView(newState)
		ubaDevicesDispatch(setState(newState));
		setSelectedCardsOrTableView(newState); // Save state to local storage
	  };

	return (
		<Toolbar style={{minHeight: '25px',}} sx={{
			display: 'flex',
		}}>
			
			
			<Stack  direction="row" alignItems="left" justifyContent="left" sx={{
				width: 1,
			}}>
				<Stack >
					<Typography variant="subtitle1">
						{getText('mainPage.UBA_CONFIGURED')} {ubaTotal?.configured}
					</Typography>
				</Stack>
			</Stack>

			<Stack  direction="row" alignItems="left" justifyContent="left" sx={{
				width: 1,
			}}>
				<Stack >
					<Typography variant="subtitle1">
						{getText('mainPage.UBA_CONNECTED')} {ubaTotal?.connected}
					</Typography>
				</Stack>
			</Stack>

			<Stack  direction="row" alignItems="left" justifyContent="left" sx={{
				width: 1,
			}}>
				<Stack >
					<Typography variant="subtitle1">
						{getText('mainPage.UBA_RUNNING')} {ubaTotal?.running}
					</Typography>
				</Stack>
			</Stack>
			<Tooltip title={cardsOrTableView === pageStateList.CARDS_VIEW ? getText('mainPage.TABLE_VIEW') : getText('mainPage.CARDS_VIEW')}>
				<IconButton onClick={handleToggle}>
					{ cardsOrTableView === pageStateList.CARDS_VIEW ? 
						<TableViewIcon fontSize="large" color="primary"/> :
						<GridViewIcon fontSize="large" color="primary"/>
					}
				</IconButton>
			</Tooltip>
		</Toolbar>
	);
}

CustomToolbar.propTypes = {
	numSelected: PropTypes.number,
	filterName: PropTypes.string,
	onFilterName: PropTypes.func,
};

import PropTypes from 'prop-types';

import Box from '@mui/material/Box';
import TableRow from '@mui/material/TableRow';
import TableHead from '@mui/material/TableHead';
import TableCell from '@mui/material/TableCell';
import TableSortLabel from '@mui/material/TableSortLabel';
import {visuallyHidden} from '@mui/utils';

import {validateString,} from 'src/utils/validators';

export default function CustomTableHead(props) {

	const {order, setOrder, orderBy, setOrderBy, headLabels,} = props;

	const handleSort = field => {
		let orderValue = 'asc';
		if (orderBy === field && order === 'asc') {
			orderValue = 'desc';
		}

		if (validateString(field)) {
			setOrder(orderValue);
			setOrderBy(field);
		}
	};

	return (
		<TableHead>
			<TableRow>
				{headLabels.map((headCell, key) => (
					<TableCell
						key={key}
						align={headCell.align || 'left'}
						sortDirection={orderBy === headCell.id ? order : false}
						sx={{width: headCell.width, minWidth: headCell.minWidth}}
					>
						{
							validateString(headCell?.id) ?
								<TableSortLabel
									hideSortIcon
									active={orderBy === headCell.id}
									direction={orderBy === headCell.id ? order : 'asc'}
									onClick={() => handleSort(headCell.id)}
								>
									{headCell.label}
									{orderBy === headCell.id ? (
										<Box sx={{...visuallyHidden}}>
											{order === 'desc' ? 'sorted descending' : 'sorted ascending'}
										</Box>
									) : null}
								</TableSortLabel> :
								headCell.label
						}
					</TableCell>
				))}
			</TableRow>
		</TableHead>
	);
}

CustomTableHead.propTypes = {
	order: PropTypes.oneOf(['asc', 'desc']),
	orderBy: PropTypes.string,
	headLabel: PropTypes.array,
	onRequestSort: PropTypes.func,
	onSelectAllClick: PropTypes.func,
};

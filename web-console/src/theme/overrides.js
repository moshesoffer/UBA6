import { outlinedInputClasses } from '@mui/material/OutlinedInput';
import { alpha } from '@mui/material/styles';

/* eslint-disable */

export function overrides(theme) {
	return {
		MuiCssBaseline: {
			styleOverrides: {
				'*': {
					boxSizing: 'border-box',
				},
				html: {
					margin: 0,
					padding: 0,
					width: '100%',
					height: '100%',
					WebkitOverflowScrolling: 'touch',
				},
				body: {
					margin: 0,
					padding: 0,
					width: '100%',
					height: '100%',
				},
				'#root': {
					width: '100%',
					height: '100%',
				},
				input: {
					'&[type=number]': {
						MozAppearance: 'textfield',
						'&::-webkit-outer-spin-button': {
							margin: 0,
							WebkitAppearance: 'none',
						},
						'&::-webkit-inner-spin-button': {
							margin: 0,
							WebkitAppearance: 'none',
						},
					},
				},
				img: {
					maxWidth: '100%',
					display: 'inline-block',
					verticalAlign: 'bottom',
				},
			},
		},
		MuiBackdrop: {
			styleOverrides: {
				root: {
					backgroundColor: alpha(theme.palette.grey[900], 0.8),
				},
				invisible: {
					background: 'transparent',
				},
			},
		},
		MuiButton: {
			styleOverrides: {
				containedInherit: {
					color: theme.palette.common.white,
					backgroundColor: theme.palette.grey[800],
					'&:hover': {
						color: theme.palette.common.white,
						backgroundColor: theme.palette.grey[800],
					},
				},
				sizeLarge: {
					minHeight: 48,
				},
			},
		},
		MuiCard: {
			styleOverrides: {
				root: {
					boxShadow: theme.customShadows.card,
					borderRadius: Number(theme.shape.borderRadius) * 2,
					position: 'relative',
					// Fix Safari overflow: hidden with border radius
					zIndex: 0,
				},
			},
		},
		MuiCardHeader: {
			styleOverrides: {
				root: {
					padding: '0px',
				},
				content: {
					flex: '1 1 auto',
					
				}
			},
		},
		MuiOutlinedInput: {
			styleOverrides: {
				root: {
					[`& .${outlinedInputClasses.notchedOutline}`]: {
						borderColor: alpha(theme.palette.grey[500], 0.24),
					},
				},
				input: {
					padding: '6.5px 14px',
				}
			},
		},
		/*MuiSelect: {
			styleOverrides: {
				select: {
					padding: '6.5px 14px',
				}
			}
		},*/
		MuiPaper: {
			defaultProps: {
				elevation: 0,
			},
		},
		/*MuiInputBase: {
			styleOverrides: {
				input: {
					padding: '12px 14px',
				}

			},
		},*/
		MuiTableCell: {
			styleOverrides: {
				root: {
					fontSize: '0.815rem',
					//lineHeight: 1.57,
					height: '45px',
				},
				head: {
					color: theme.palette.text.secondary,
					backgroundColor: theme.palette.background.neutral,
					padding: '5px'
				},
				body: {
					padding: '1px 0px 0px 5px',
					//fontSize: '0.775rem'
				}
			},
		},
		MuiTooltip: {
			styleOverrides: {
				tooltip: {
					backgroundColor: theme.palette.grey[800],
				},
				arrow: {
					color: theme.palette.grey[800],
				},
			},
		},
		MuiTypography: {
			styleOverrides: {
				paragraph: {
					marginBottom: theme.spacing(2),
				},
				gutterBottom: {
					marginBottom: theme.spacing(1),
				},
			},
		},
		MuiMenuItem: {
			styleOverrides: {
				root: {
					...theme.typography.body2,
				},
			},
		},
	};
}

/* eslint-enable */

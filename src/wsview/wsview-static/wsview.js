function scale_min(a) {
	return 5 * parseInt((a + 4) / 5) - 5;
}

function scale_max(a) {
	return 5 * parseInt(a / 5) + 5;
}

function time_get(row) {
	return new Date(row.time);
}

function time_start(date, period) {
	if (period == 'month') {
		date.setDate(1);
	} else if (period == 'year') {
		date.setMonth(0);
	}
}

function time_end(date, period) {
	if (period == 'month') {
		date.setDate(1);
		date.setMonth(date.getMonth() + 1);
	} else if (period == 'year') {
		date.setMonth(0);
		date.setYear(date.getYear() + 1);
	}
}

function time_next(date, period) {
	if (period == 'month') {
		date.setDate(date.getDate() + 1);
	} else if (period = 'year') {
		date.setMonth(date.getMonth() + 1);
	}
}

function get_labels(json, period) {
	var labels = [];

	var x = time_get(json[0]);
	var last = time_get(json[json.length-1]);

	time_start(x, period);
	time_end(last, period);

	for (var i = 0; i < json.length; i++) {
		var time = time_get(json[i]);

		while (x.getTime() < time.getTime()) {
			labels.push(new Date(x));
			time_next(x, period);
		}

		labels.push(time);
		time_next(x, period);
	}

	while (x.getTime() < last.getTime()) {
		labels.push(new Date(x));
		time_next(x, period);
	}

	return labels;
}

function get_data(json, field, period) {
	var data = [];

	var x = time_get(json[0]);

	time_start(x, period);

	for (var i = 0; i < json.length; i++) {
		var time = time_get(json[i]);

		while (x.getTime() < time.getTime()) {
			data.push(null);
			time_next(x, period);
		}

		data.push(json[i][field]);
		time_next(x, period);
	}

	return data;
}

function get_type(config) {
	var type = 'line';
	var datasets = config.datasets;

	for (i = 0; i < datasets.length; i++) {
		if (datasets[i].type != 'line') {
			type = datasets[i].type;
		}
	}

	return type;
}

function create_chart(json, config, period) {
	var time_unit;
	var title_fmt;
	var time_step;

	if (period == 'month') {
		time_unit = 'day';
		time_step = 2;
		title_fmt = 'D MMMM YYYY';
	} else if (period == 'year') {
		time_unit = 'month';
		time_step = 1;
		title_fmt = 'MMMM YYYY';
	}

	var chartjs = {
		type: get_type(config),
		data: {
			labels: get_labels(json, period),
			datasets: [],
		},
		options: {
			title: {
				display: true,
				fontSize: 16,
				text: config.options.title
			},
			legend: {
				position: 'bottom'
			},
			tooltips: {
				mode: 'index',
				intersect: false,
				position: 'nearest',
				bodySpacing: 5,
				callbacks: {
					title: function(items, data) {
						return moment(items[0].xLabel).format(title_fmt);
					},
					label: function(item, data) {
						var result = null;
						var dataset = data.datasets[item.datasetIndex];

						if (dataset.data[item.index] != null) {
							var unit = '';

							for (i = 0; i < config.options.axes.length; i++) {
								if (dataset.yAxisID == config.options.axes[i].id) {
									unit = config.options.axes[i].unit;
								}
							}

							result = dataset.label + ' : ' + item.yLabel.toFixed(1) + ' ' + unit;
						}

						return result;
					}
				}
			},
			scales: {
				xAxes: [{
					type: 'time',
					gridLines: {
						offsetGridLines: false
					},
					time: {
						unit: time_unit,
						parser: moment,
						stepSize: time_step,
//						min: time_min,
						displayFormats: {
							hour: 'HH:mm',
							day: 'D',
							month: 'MMM'
						}
					},
					scaleLabel: {
						display: true,
//						labelString: x_label,
						fontStyle: 'bold'
					}
				}],
				yAxes: []
			}
		}
	}

	// Configure datasets
	for (i = 0; i < config.datasets.length; i++) {
		var dataset = config.datasets[i];

		var dat = {
			type: dataset.type,
			label: dataset.label,
			yAxisID: dataset.axis,
			backgroundColor: dataset.color,
			borderColor: dataset.color,
			data: get_data(json, dataset.field, period)
		};

		if (dataset.type == 'line') {
			dat.fill = false;
			dat.lineTension = 0;
			dat.borderWidth = 2;
			dat.pointStyle = dataset.pointStyle;
//			dat.pointRadius = 0;
//			dat.pointHitRadius = 5;
//			dat.pointHoverRadius = 5;
			dat.spanGaps = false;
		}

		chartjs.data.datasets.push(dat);
	}

	// Configure axes
	for (i = 0; i < config.options.axes.length; i++) {
		var axes = config.options.axes[i];

		var dat = {
			type: 'linear',
			position: axes.position,
			id: axes.id,
			ticks: {
//				min: scale_min(temp_min.min),
//				max: scale_max(temp_max.max),
				stepSize: 5
			},
			scaleLabel: {
				display: true,
				labelString: axes.label + ' (' + axes.unit + ')',
				fontStyle: 'bold'
			}
		};

		chartjs.options.scales.yAxes.push(dat);
	}

	return chartjs;
}

function obs_temp(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température',
			field: 'temp',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(237, 86, 27, 1)'
		},{
			type: 'line',
			label: 'Point de rosée',
			field: 'dew_point',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(80, 180, 50, 1)'
		},{
			type: 'line',
			label: 'Humidité',
			field: 'humidity',
			axis: 'y-axis-2',
			color: 'rgba(5, 141, 199, 1)'
		}],
		options: {
			title: 'Températures, humidité, point de rosée',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Température',
				unit: '°C'
			},{
				id: 'y-axis-2',
				position: 'right',
				label: 'Humidité',
				unit: '%'
			}]
		}
	};

	return create_chart(json, options);
}

function obs_wind(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Vent moyen',
			field: 'wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(69, 114, 167, 1)'
		},{
			type: 'line',
			label: 'Direction',
			field: 'wind_dir',
			axis: 'y-axis-2',
			pointStyle: 'rect',
			color: 'rgba(170, 70, 70, 1)'
		}],
		options: {
			title: 'Vent',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Vent',
				unit: 'm/s'
			},{
				id: 'y-axis-2',
				position: 'right',
				label: 'Direction',
				unit: '°'
			}]
		}
	};

	return create_chart(json, options);
}

function chart_wind(json, period) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Rafale',
			field: 'hi_wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(86, 65, 112, 1)'
		},{
			type: 'line',
			label: 'Vent moyen',
			field: 'avg_wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(128, 105, 155, 1)'
		}],
		options: {
			title: 'Vent',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Vent',
				unit: 'm/s'
			}]
		}
	};

	return create_chart(json, options, period);
};

function chart_barometer(json, period) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Pression moyenne',
			field: 'barometer',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(137, 165, 78, 1)'
		}],
		options: {
			title: 'Pression au niveau de la mer',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Pression',
				unit: 'hPa'
			}]
		}
	};

	return create_chart(json, options, period);
};

function chart_temp(json, period) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température maximale',
			field: 'hi_temp',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(170, 70, 70, 1)'
		},{
			type: 'line',
			label: 'Température minimale',
			field: 'lo_temp',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(69, 114, 167, 1)'
		}],
		options: {
			title: 'Températures',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Températures',
				unit: '°C'
			}]
		}
	};

	return create_chart(json, options, period);
};

function chart_temp_rain(json, period) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température maximale',
			field: 'hi_temp',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(170, 70, 70, 1)'
		},{
			type: 'line',
			label: 'Température minimale',
			field: 'lo_temp',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(69, 114, 167, 1)'
		},{
			type: 'bar',
			label: 'Précipitations',
			field: 'rain_fall',
			axis: 'y-axis-2',
			color: 'rgba(162, 190, 163, 1)'
		}],
		options: {
			title: 'Températures',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Températures',
				unit: '°C'
			},{
				id: 'y-axis-2',
				position: 'right',
				label: 'Précipitations',
				unit: 'mm'
			}]
		}
	};

	return create_chart(json, options, period);
};

function chart_rain(json, period) {
	var options = {
		datasets: [{
			type: 'bar',
			label: 'Précipitations',
			field: 'rain',
			axis: 'y-axis-1',
			color: 'rgba(162, 190, 163, 1)'
		},{
			type: 'line',
			label: 'Précipitations maximales sur 24h',
			field: 'rain_24h',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(18, 137, 186, 1)'
		}],
		options: {
			title: 'Précipitations',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Précipitations',
				unit: 'mm'
			}]
		}
	};

	return create_chart(json, options, period);
};


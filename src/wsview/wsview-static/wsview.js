function capitalize(s) {
	return s.charAt(0).toUpperCase() + s.slice(1);
}

function parse(t) {
	if (typeof(t) == 'number') {
		t = moment.unix(t);
	} else {
		t = moment(t);
	}

	return t;
}

function time_next(t, unit) {
	t.add(1, unit);
}

// Compute additional metrics
function compute(data) {
	var v = 0;

	for (var i = 0; i < data.length; i++) {
		v += data[i].rain;
		data[i].rain_sum = v;
	}
}

function get_labels(json) {
	var labels = [];

	var x = parse(json.from);
	var last = parse(json.to);

	for (var i = 0; i < json.data.length; i++) {
		var time = parse(json.data[i].time);

		while (x.isBefore(time)) {
			labels.push(x.clone());
			time_next(x, json.unit);
		}

		labels.push(time);
		time_next(x, json.unit);
	}

	while (x.isBefore(last)) {
		labels.push(x.clone());
		time_next(x, json.unit);
	}

	return labels;
}

function get_data(json, field) {
	var data = [];

	var x = parse(json.from);

	for (var i = 0; i < json.data.length; i++) {
		var time = parse(json.data[i].time);

		while (x.isBefore(time)) {
			data.push(null);
			time_next(x, json.unit);
		}

		data.push(json.data[i][field]);
		time_next(x, json.unit);
	}

	return data;
}

// TODO use same order than legend
function tooltip_label(config, item, data) {
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

function create_chart(json, config) {
	var time_unit;
	var title_fmt;
	var time_step;

	if (json.unit == null) {
	} else if (json.unit == 'day') {
		time_unit = 'day';
		time_step = 2;
		title_fmt = 'D MMMM YYYY';
	} else if (json.unit == 'month') {
		time_unit = 'month';
		time_step = 1;
		title_fmt = 'MMMM YYYY';
	}

	var chartjs = {
		type: 'bar',
		data: {
			labels: get_labels(json),
			datasets: [],
		},
		options: {
			title: {
				display: true,
				fontSize: 16,
				text: config.options.title
			},
			legend: {
				position: 'bottom',
				reverse: config.options.reverse
			},
			tooltips: {
				mode: 'index',
				intersect: false,
				position: 'nearest',
				bodySpacing: 5,
				callbacks: {
					label: function(item, data) {
						return tooltip_label(config, item, data);
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
						stepSize: time_step,
						tooltipFormat: title_fmt,
						displayFormats: {
							hour: 'HH:mm',
							day: 'D',
							month: 'MMM'
						}
					},
					scaleLabel: {
						display: true,
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
			data: get_data(json, dataset.field)
		};

		if (dataset.type == 'line') {
			dat.fill = false;
			dat.spanGaps = false;
			dat.lineTension = 0;
			dat.borderWidth = 2;
			dat.pointStyle = dataset.pointStyle;
			if (dataset.pointHide) {
				dat.pointRadius = 0;
				dat.pointHitRadius = 4;
				dat.pointHoverRadius = 4;
			}
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

function archive_temp(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température',
			field: 'temp',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			pointHide: 1,
			color: 'rgba(237, 86, 27, 1)'
		},{
			type: 'line',
			label: 'Point de rosée',
			field: 'dew_point',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			pointHide: 1,
			color: 'rgba(80, 180, 50, 1)'
		},{
			type: 'line',
			label: 'Humidité',
			field: 'humidity',
			axis: 'y-axis-2',
			pointHide: 1,
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

function archive_wind(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Vent moyen',
			field: 'avg_wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			pointHide: 1,
			color: 'rgba(69, 114, 167, 1)'
		},{
			type: 'bubble',
			label: 'Direction',
			field: 'avg_wind_dir',
			axis: 'y-axis-2',
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

function aggr_wind(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Vent moyen',
			field: 'avg_wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(128, 105, 155, 1)'
		},{
			type: 'line',
			label: 'Rafale',
			field: 'hi_wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(86, 65, 112, 1)'
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

	return create_chart(json, options);
};

function aggr_barometer(json) {
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

	return create_chart(json, options);
};

function aggr_temp(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température minimale',
			field: 'lo_temp',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(69, 114, 167, 1)'
		},{
			type: 'line',
			label: 'Température maximale',
			field: 'hi_temp',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(170, 70, 70, 1)'
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

	return create_chart(json, options);
};

function aggr_temp_rain(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température minimale',
			field: 'lo_temp',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(69, 114, 167, 1)'
		},{
			type: 'line',
			label: 'Température maximale',
			field: 'hi_temp',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(170, 70, 70, 1)'
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

	return create_chart(json, options);
};

function aggr_rain(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Cumul annuel',
			field: 'rain_sum',
			axis: 'y-axis-2',
			pointStyle: 'rect',
			color: 'rgba(0, 0, 0, 1)'
		},{
			type: 'line',
			label: 'Précipitations maximales sur 24h',
			field: 'rain_24h',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(18, 137, 186, 1)'
		},{
			type: 'bar',
			label: 'Précipitations',
			field: 'rain',
			axis: 'y-axis-1',
			color: 'rgba(162, 190, 163, 1)'
		}],
		options: {
			title: 'Précipitations',
			reverse: true,
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Précipitations',
				unit: 'mm'
			},{
				id: 'y-axis-2',
				position: 'right',
				label: 'Cumul annuel',
				unit: 'mm'
			}]
		}
	};

	compute(json.data);

	return create_chart(json, options);
};

function tr_cell_val(tr, value) {
	var td = tr.insertCell();

	td.appendChild(document.createTextNode(value));
}

function tr_cell(tr, row, field) {
	var td = tr.insertCell();
	var v = row[field.field];

	if (v != null) {
		v = v.toFixed(1) + ' ' + field.unit;
	} else {
		v = '';
	}

	td.appendChild(document.createTextNode(v));
}

function mk_table(root, json, fields) {
	var title_fmt;

	if (json.unit == null) {
	} else if (json.unit == 'day') {
		title_fmt = 'dddd D';
	} else if (json.unit == 'month') {
		title_fmt = 'MMMM';
	}

	var body = document.body;
	var tbl = document.createElement('table');

	for (var i = 0; i < json.data.length; i++) {
		var tr = tbl.insertRow();
		var time = parse(json.data[i].time);

		tr_cell_val(tr, capitalize(time.format(title_fmt)));
		for (var j = 0; j < fields.length; j++) {
			tr_cell(tr, json.data[i], fields[j]);
		}
	}

	root.appendChild(tbl);
}

function table_archive(root, json) {
	var fields = [ 'temp' ];

	mk_table(root, json, fields);
}

function table_aggr(root, json) {
	var fields = [{
		field: 'lo_temp',
		unit: '°C'
	},{
		field: 'hi_temp',
		unit: '°C'
	},{
		field: 'rain_fall',
		unit: 'mm'
	},{
		field: 'hi_wind_speed',
		unit: 'm/s'
	}];

	mk_table(root, json, fields);
}

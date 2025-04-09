% ==== CONFIGURACIÓN GENERAL ====
filename = 'adc_data.bin';
num_channels = 4;
num_samples_to_plot = 5120;
channel_names = {'CH0', 'CH1', 'CH2', 'CH3'};

% ==== CONFIGURACIÓN DE VISUALIZACIÓN ====
% Opciones para seleccionar qué canales graficar:
% - Para ver TODOS los canales: selected_channels = 1:4;
% - Para ver SOLO un canal (por ejemplo, CH0): selected_channels = 1;
% - Para ver dos canales (por ejemplo, CH2 y CH3): selected_channels = [3 4];
selected_channels = 4;  % <- MODIFICAR ESTA LÍNEA SEGÚN NECESIDAD

% Estilo de figura para publicación científica
figure_size = [800 400];            % Tamaño en píxeles [ancho, alto]
line_width = 1.2;                   % Grosor de línea de las curvas
marker_size = 12;                    % Tamaño del marcador
font_size = 12;                     % Tamaño de fuente de ejes y título
export_filename = 'adc_plot.png';   % Nombre del archivo a guardar
export_dpi = 300;                   % Resolución para guardar (DPI)

% ==== LECTURA DE ARCHIVO BINARIO ====
fid = fopen(filename, 'rb');
if fid == -1
    error('No se pudo abrir el archivo: %s', filename);
end

raw_data = fread(fid, 'uint16');
fclose(fid);

% ==== ORGANIZAR DATOS ====
total_samples = floor(length(raw_data) / num_channels);
data = reshape(raw_data(1:total_samples * num_channels), num_channels, [])';
fprintf('Muestras por canal: %d\n', size(data, 1));

% ==== GRAFICAR ====
n = min(num_samples_to_plot, size(data, 1));
plot_data = data(1:n, selected_channels);

% Nombres de los canales seleccionados
selected_names = channel_names(selected_channels);

% Crear figura para graficar
f = figure('Name', 'Datos ADC', 'Color', 'w', 'Units', 'pixels', ...
           'Position', [100 100 figure_size]);

plot(plot_data, '.-', 'LineWidth', line_width, 'MarkerSize', marker_size);
grid on;
title('Datos ADC por canal', 'FontSize', font_size + 2);
xlabel('Índice de muestra', 'FontSize', font_size);
ylabel('Valor ADC', 'FontSize', font_size);
legend(selected_names, 'FontSize', font_size);
set(gca, 'FontSize', font_size);

% ==== GUARDAR IMAGEN ====
saveas(f, export_filename);
% Alternativa para exportar con mayor control de calidad:
% print(f, export_filename, '-dpng', sprintf('-r%d', export_dpi));
